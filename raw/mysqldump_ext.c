// mysqldump_ext.c
#include "php.h"
#include "ext/standard/info.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PHP_MYSQLDUMP_EXT_VERSION "1.0"

// Forward declaration of module info function
PHP_MINFO_FUNCTION(mysqldump_ext);

// Function declarations
PHP_FUNCTION(mysqldump_exec);

// Module entry
zend_function_entry mysqldump_ext_functions[] = {
    PHP_FE(mysqldump_exec, NULL)
    PHP_FE_END
};

zend_module_entry mysqldump_ext_module_entry = {
    STANDARD_MODULE_HEADER,
    "mysqldump_ext",
    mysqldump_ext_functions,
    NULL, // PHP_MINIT
    NULL, // PHP_MSHUTDOWN
    NULL, // PHP_RINIT
    NULL, // PHP_RSHUTDOWN
    PHP_MINFO(mysqldump_ext),
    PHP_MYSQLDUMP_EXT_VERSION,
    STANDARD_MODULE_PROPERTIES
};

ZEND_GET_MODULE(mysqldump_ext)

// Module info function
PHP_MINFO_FUNCTION(mysqldump_ext) {
    php_info_print_table_start();
    php_info_print_table_header(2, "MySQLDump Extension", "enabled");
    php_info_print_table_row(2, "Version", PHP_MYSQLDUMP_EXT_VERSION);
    php_info_print_table_end();
}

// Helper functions for escaping and command building
static char* escape_shell_arg(const char *str) {
    if (str == NULL) return NULL;
    
    size_t len = strlen(str);
    size_t newlen = 2 * len + 3; // Worst case: every char needs escaping + quotes + null
    
    char *escaped = (char*)malloc(newlen);
    if (escaped == NULL) return NULL;
    
    char *dst = escaped;
    *dst++ = '\'';
    
    for (size_t i = 0; i < len; i++) {
        if (str[i] == '\'') {
            *dst++ = '\'';
            *dst++ = '\\';
            *dst++ = '\'';
            *dst++ = '\'';
        } else {
            *dst++ = str[i];
        }
    }
    
    *dst++ = '\'';
    *dst = '\0';
    
    return escaped;
}

// Implementation of mysqldump_exec function
PHP_FUNCTION(mysqldump_exec) {
    char *host = NULL, *user = NULL, *pass = NULL, *db = NULL, *output_file = NULL;
    size_t host_len, user_len, pass_len, db_len, output_len;
    zend_long port = 3306;
    zval *options = NULL;
    
    // Parse parameters (host, user, pass, db, port, output_file, options_array)
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ssssl|sa", 
                             &host, &host_len, 
                             &user, &user_len, 
                             &pass, &pass_len, 
                             &db, &db_len, 
                             &port, 
                             &output_file, &output_len,
                             &options) == FAILURE) {
        RETURN_FALSE;
    }
    
    // Start building command with base options
    char command[8192]; // Increased buffer size for the command
    snprintf(command, sizeof(command), "mysqldump -h%s -u%s -p%s -P%ld %s", 
             host, user, pass, port, db);
    
    // Process additional options if provided
    if (options != NULL && Z_TYPE_P(options) == IS_ARRAY) {
        HashTable *options_hash = Z_ARRVAL_P(options);
        zval *option_val;
        zend_string *option_key;
        
        ZEND_HASH_FOREACH_STR_KEY_VAL(options_hash, option_key, option_val) {
            if (option_key) {
                const char *key = ZSTR_VAL(option_key);
                
                // Boolean flags (no value)
                if (Z_TYPE_P(option_val) == IS_TRUE) {
                    // Add supported boolean flags
                    if (strcmp(key, "all-databases") == 0 || strcmp(key, "A") == 0) {
                        strcat(command, " --all-databases");
                    } else if (strcmp(key, "all-tablespaces") == 0 || strcmp(key, "Y") == 0) {
                        strcat(command, " --all-tablespaces");
                    } else if (strcmp(key, "no-tablespaces") == 0 || strcmp(key, "y") == 0) {
                        strcat(command, " --no-tablespaces");
                    } else if (strcmp(key, "add-drop-database") == 0) {
                        strcat(command, " --add-drop-database");
                    } else if (strcmp(key, "add-drop-table") == 0) {
                        strcat(command, " --add-drop-table");
                    } else if (strcmp(key, "add-drop-trigger") == 0) {
                        strcat(command, " --add-drop-trigger");
                    } else if (strcmp(key, "add-locks") == 0) {
                        strcat(command, " --add-locks");
                    } else if (strcmp(key, "allow-keywords") == 0) {
                        strcat(command, " --allow-keywords");
                    } else if (strcmp(key, "apply-replica-statements") == 0) {
                        strcat(command, " --apply-replica-statements");
                    } else if (strcmp(key, "comments") == 0 || strcmp(key, "i") == 0) {
                        strcat(command, " --comments");
                    } else if (strcmp(key, "compact") == 0) {
                        strcat(command, " --compact");
                    } else if (strcmp(key, "complete-insert") == 0 || strcmp(key, "c") == 0) {
                        strcat(command, " --complete-insert");
                    } else if (strcmp(key, "compress") == 0 || strcmp(key, "C") == 0) {
                        strcat(command, " --compress");
                    } else if (strcmp(key, "create-options") == 0 || strcmp(key, "a") == 0) {
                        strcat(command, " --create-options");
                    } else if (strcmp(key, "databases") == 0 || strcmp(key, "B") == 0) {
                        strcat(command, " --databases");
                    } else if (strcmp(key, "delete-source-logs") == 0) {
                        strcat(command, " --delete-source-logs");
                    } else if (strcmp(key, "disable-keys") == 0 || strcmp(key, "K") == 0) {
                        strcat(command, " --disable-keys");
                    } else if (strcmp(key, "events") == 0 || strcmp(key, "E") == 0) {
                        strcat(command, " --events");
                    } else if (strcmp(key, "extended-insert") == 0 || strcmp(key, "e") == 0) {
                        strcat(command, " --extended-insert");
                    } else if (strcmp(key, "flush-logs") == 0 || strcmp(key, "F") == 0) {
                        strcat(command, " --flush-logs");
                    } else if (strcmp(key, "flush-privileges") == 0) {
                        strcat(command, " --flush-privileges");
                    } else if (strcmp(key, "force") == 0 || strcmp(key, "f") == 0) {
                        strcat(command, " --force");
                    } else if (strcmp(key, "hex-blob") == 0) {
                        strcat(command, " --hex-blob");
                    } else if (strcmp(key, "include-source-host-port") == 0) {
                        strcat(command, " --include-source-host-port");
                    } else if (strcmp(key, "insert-ignore") == 0) {
                        strcat(command, " --insert-ignore");
                    } else if (strcmp(key, "lock-all-tables") == 0 || strcmp(key, "x") == 0) {
                        strcat(command, " --lock-all-tables");
                    } else if (strcmp(key, "lock-tables") == 0 || strcmp(key, "l") == 0) {
                        strcat(command, " --lock-tables");
                    } else if (strcmp(key, "no-autocommit") == 0) {
                        strcat(command, " --no-autocommit");
                    } else if (strcmp(key, "no-create-db") == 0 || strcmp(key, "n") == 0) {
                        strcat(command, " --no-create-db");
                    } else if (strcmp(key, "no-create-info") == 0 || strcmp(key, "t") == 0) {
                        strcat(command, " --no-create-info");
                    } else if (strcmp(key, "no-data") == 0 || strcmp(key, "d") == 0) {
                        strcat(command, " --no-data");
                    } else if (strcmp(key, "no-set-names") == 0 || strcmp(key, "N") == 0) {
                        strcat(command, " --no-set-names");
                    } else if (strcmp(key, "opt") == 0) {
                        strcat(command, " --opt");
                    } else if (strcmp(key, "order-by-primary") == 0) {
                        strcat(command, " --order-by-primary");
                    } else if (strcmp(key, "quick") == 0 || strcmp(key, "q") == 0) {
                        strcat(command, " --quick");
                    } else if (strcmp(key, "quote-names") == 0 || strcmp(key, "Q") == 0) {
                        strcat(command, " --quote-names");
                    } else if (strcmp(key, "replace") == 0) {
                        strcat(command, " --replace");
                    } else if (strcmp(key, "routines") == 0 || strcmp(key, "R") == 0) {
                        strcat(command, " --routines");
                    } else if (strcmp(key, "set-charset") == 0) {
                        strcat(command, " --set-charset");
                    } else if (strcmp(key, "single-transaction") == 0) {
                        strcat(command, " --single-transaction");
                    } else if (strcmp(key, "dump-date") == 0) {
                        strcat(command, " --dump-date");
                    } else if (strcmp(key, "skip-opt") == 0) {
                        strcat(command, " --skip-opt");
                    } else if (strcmp(key, "get-server-public-key") == 0) {
                        strcat(command, " --get-server-public-key");
                    } else if (strcmp(key, "tables") == 0) {
                        strcat(command, " --tables");
                    } else if (strcmp(key, "triggers") == 0) {
                        strcat(command, " --triggers");
                    } else if (strcmp(key, "tz-utc") == 0) {
                        strcat(command, " --tz-utc");
                    } else if (strcmp(key, "verbose") == 0 || strcmp(key, "v") == 0) {
                        strcat(command, " --verbose");
                    } else if (strcmp(key, "xml") == 0 || strcmp(key, "X") == 0) {
                        strcat(command, " --xml");
                    } else if (strcmp(key, "enable-cleartext-plugin") == 0) {
                        strcat(command, " --enable-cleartext-plugin");
                    } else if (strcmp(key, "network-timeout") == 0 || strcmp(key, "M") == 0) {
                        strcat(command, " --network-timeout");
                    } else if (strcmp(key, "show-create-table-skip-secondary-engine") == 0) {
                        strcat(command, " --show-create-table-skip-secondary-engine");
                    } else if (strcmp(key, "skip-generated-invisible-primary-key") == 0) {
                        strcat(command, " --skip-generated-invisible-primary-key");
                    }
                }
                // Options with values
                else if (Z_TYPE_P(option_val) == IS_STRING || Z_TYPE_P(option_val) == IS_LONG || Z_TYPE_P(option_val) == IS_DOUBLE) {
                    char option_buff[1024];
                    
                    if (Z_TYPE_P(option_val) == IS_STRING) {
                        char *escaped_val = escape_shell_arg(Z_STRVAL_P(option_val));
                        snprintf(option_buff, sizeof(option_buff), " --%s=%s", key, escaped_val);
                        free(escaped_val);
                    } else if (Z_TYPE_P(option_val) == IS_LONG) {
                        snprintf(option_buff, sizeof(option_buff), " --%s=%ld", key, Z_LVAL_P(option_val));
                    } else if (Z_TYPE_P(option_val) == IS_DOUBLE) {
                        snprintf(option_buff, sizeof(option_buff), " --%s=%f", key, Z_DVAL_P(option_val));
                    }
                    
                    strcat(command, option_buff);
                }
            }
        } ZEND_HASH_FOREACH_END();
    }
    
    // Add output redirection if output file is provided
    if (output_file != NULL && output_len > 0) {
        char *escaped_output = escape_shell_arg(output_file);
        char output_buff[1024];
        snprintf(output_buff, sizeof(output_buff), " > %s", escaped_output);
        strcat(command, output_buff);
        free(escaped_output);
    }
    
    // Execute command
    int result = system(command);
    
    if (result == 0) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}