<?php
/**
 * MySQLDumpExt - PHP Wrapper Class for mysqldump_ext extension
 */
class MySQLDumpExt {
    private $host;
    private $username;
    private $password;
    private $database;
    private $port;
    private $output_file;
    private $options = [];
    
    /**
     * Constructor
     * 
     * @param string $host Database hostname
     * @param string $username Database username
     * @param string $password Database password
     * @param string $database Database name
     * @param int $port Database port (default: 3306)
     * @param string|null $output_file Output file name (default: auto-generated)
     */
    public function __construct($host, $username, $password, $database, $port = 3306, $output_file = null) {
        $this->host = $host;
        $this->username = $username;
        $this->password = $password;
        $this->database = $database;
        $this->port = $port;
        $this->output_file = $output_file ?: 'backup_' . date('Y-m-d_H-i-s') . '.sql';
    }
    
    /**
     * Set a flag option
     * 
     * @param string $option Option name
     * @param bool $value Option value (default: true)
     * @return $this
     */
    public function setFlag($option, $value = true) {
        $this->options[$option] = $value ? true : false;
        return $this;
    }
    
    /**
     * Set an option with a value
     * 
     * @param string $option Option name
     * @param mixed $value Option value
     * @return $this
     */
    public function setOption($option, $value) {
        $this->options[$option] = $value;
        return $this;
    }
    
    /**
     * Dump all databases
     * 
     * @return $this
     */
    public function allDatabases() {
        return $this->setFlag('all-databases');
    }
    
    /**
     * Add DROP TABLE statements
     * 
     * @return $this
     */
    public function addDropTable() {
        return $this->setFlag('add-drop-table');
    }
    
    /**
     * Add locks around INSERT statements
     * 
     * @return $this
     */
    public function addLocks() {
        return $this->setFlag('add-locks');
    }
    
    /**
     * Do not include data (structure only)
     * 
     * @return $this
     */
    public function noData() {
        return $this->setFlag('no-data');
    }
    
    /**
     * Include specific tables
     * 
     * @return $this
     */
    public function withTables() {
        return $this->setFlag('tables');
    }
    
    /**
     * Include stored routines (procedures and functions)
     * 
     * @return $this
     */
    public function withRoutines() {
        return $this->setFlag('routines');
    }
    
    /**
     * Include triggers
     * 
     * @return $this
     */
    public function withTriggers() {
        return $this->setFlag('triggers');
    }
    
    /**
     * Include events
     * 
     * @return $this
     */
    public function withEvents() {
        return $this->setFlag('events');
    }
    
    /**
     * Use a single transaction for the dump
     * 
     * @return $this
     */
    public function singleTransaction() {
        return $this->setFlag('single-transaction');
    }
    
    /**
     * Lock tables during the dump
     * 
     * @return $this
     */
    public function lockTables() {
        return $this->setFlag('lock-tables');
    }
    
    /**
     * Set character set
     * 
     * @param string $charset Character set name
     * @return $this
     */
    public function setCharset($charset) {
        return $this->setOption('default-character-set', $charset);
    }
    
    /**
     * Ignore specific tables during the dump
     * 
     * @param string $table Table name to ignore
     * @return $this
     */
    public function ignoreTable($table) {
        if (isset($this->options['ignore-table']) && is_array($this->options['ignore-table'])) {
            $this->options['ignore-table'][] = $table;
        } else {
            $this->options['ignore-table'] = [$table];
        }
        return $this;
    }
    
    /**
     * Add a WHERE condition for data selection
     * 
     * @param string $condition WHERE condition
     * @return $this
     */
    public function where($condition) {
        return $this->setOption('where', $condition);
    }
    
    /**
     * Use compression in server/client protocol
     * 
     * @return $this
     */
    public function compress() {
        return $this->setFlag('compress');
    }
    
    /**
     * Show version information
     * 
     * @return $this
     */
    public function version() {
        return $this->setFlag('version');
    }
    
    /**
     * Use optimized settings (same as --add-drop-table, --add-locks, etc.)
     * 
     * @return $this
     */
    public function optimized() {
        return $this->setFlag('opt');
    }
    
    /**
     * Dump binary data in hex format
     * 
     * @return $this
     */
    public function hexBlob() {
        return $this->setFlag('hex-blob');
    }
    
    /**
     * Set compatibility mode
     * 
     * @param string $mode Compatibility mode
     * @return $this
     */
    public function compatible($mode) {
        return $this->setOption('compatible', $mode);
    }
    
    /**
     * Flush logs before the dump
     * 
     * @return $this
     */
    public function flushLogs() {
        return $this->setFlag('flush-logs');
    }
    
    /**
     * Use extended INSERT statements
     * 
     * @return $this
     */
    public function extendedInsert() {
        return $this->setFlag('extended-insert');
    }
    
    /**
     * Use complete INSERT statements
     * 
     * @return $this
     */
    public function completeInsert() {
        return $this->setFlag('complete-insert');
    }
    
    /**
     * Output as XML
     * 
     * @return $this
     */
    public function xml() {
        return $this->setFlag('xml');
    }
    
    /**
     * Set the output file name
     * 
     * @param string $file Output file name
     * @return $this
     */
    public function setOutputFile($file) {
        $this->output_file = $file;
        return $this;
    }
    
    /**
     * Execute the dump
     * 
     * @return string Path to the output file
     * @throws Exception When dump fails
     */
    public function dump() {
        // Process options before execution
        $finalOptions = $this->processOptions();
        
        // This calls our C extension function
        $result = mysqldump_exec(
            $this->host,
            $this->username,
            $this->password,
            $this->database,
            $this->port,
            $this->output_file,
            $finalOptions
        );
        
        if (!$result) {
            throw new Exception("Failed to create MySQL dump");
        }
        
        return $this->output_file;
    }
    
    /**
     * Process options before passing to extension
     * 
     * @return array Processed options
     */
    private function processOptions() {
        $finalOptions = $this->options;
        
        // Process special options that need formatting
        if (isset($finalOptions['ignore-table']) && is_array($finalOptions['ignore-table'])) {
            // Flatten ignore-table array to string format
            $ignoreTable = [];
            foreach ($finalOptions['ignore-table'] as $table) {
                if (!strpos($table, '.')) {
                    $table = $this->database . '.' . $table;
                }
                $ignoreTable[] = "--ignore-table=$table";
            }
            $finalOptions['ignore-table'] = implode(' ', $ignoreTable);
        }
        
        return $finalOptions;
    }
}
?>