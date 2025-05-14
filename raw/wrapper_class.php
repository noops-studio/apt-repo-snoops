<?php
class MySQLDumpExt {
    private $host;
    private $username;
    private $password;
    private $database;
    private $port;
    private $output_file;
    private $options = [];
    
    public function __construct($host, $username, $password, $database, $port = 3306, $output_file = null) {
        $this->host = $host;
        $this->username = $username;
        $this->password = $password;
        $this->database = $database;
        $this->port = $port;
        $this->output_file = $output_file ?: 'backup_' . date('Y-m-d_H-i-s') . '.sql';
    }
    
    // Methods to set all the different options
    
    // Boolean flag options
    public function setFlag($option, $value = true) {
        $this->options[$option] = $value ? true : false;
        return $this;
    }
    
    // Value options
    public function setOption($option, $value) {
        $this->options[$option] = $value;
        return $this;
    }
    
    // Shorthand methods for common options
    public function allDatabases() {
        return $this->setFlag('all-databases');
    }
    
    public function addDropTable() {
        return $this->setFlag('add-drop-table');
    }
    
    public function addLocks() {
        return $this->setFlag('add-locks');
    }
    
    public function noData() {
        return $this->setFlag('no-data');
    }
    
    public function withTables() {
        return $this->setFlag('tables');
    }
    
    public function withRoutines() {
        return $this->setFlag('routines');
    }
    
    public function withTriggers() {
        return $this->setFlag('triggers');
    }
    
    public function withEvents() {
        return $this->setFlag('events');
    }
    
    public function singleTransaction() {
        return $this->setFlag('single-transaction');
    }
    
    public function lockTables() {
        return $this->setFlag('lock-tables');
    }
    
    public function setCharset($charset) {
        return $this->setOption('default-character-set', $charset);
    }
    
    public function ignoreTable($table) {
        if (isset($this->options['ignore-table']) && is_array($this->options['ignore-table'])) {
            $this->options['ignore-table'][] = $table;
        } else {
            $this->options['ignore-table'] = [$table];
        }
        return $this;
    }
    
    public function where($condition) {
        return $this->setOption('where', $condition);
    }
    
    public function compress() {
        return $this->setFlag('compress');
    }
    
    public function version() {
        return $this->setFlag('version');
    }
    
    public function optimized() {
        return $this->setFlag('opt');
    }
    
    public function hexBlob() {
        return $this->setFlag('hex-blob');
    }
    
    public function compatible($mode) {
        return $this->setOption('compatible', $mode);
    }
    
    public function flushLogs() {
        return $this->setFlag('flush-logs');
    }
    
    public function extendedInsert() {
        return $this->setFlag('extended-insert');
    }
    
    public function completeInsert() {
        return $this->setFlag('complete-insert');
    }
    
    public function xml() {
        return $this->setFlag('xml');
    }
    
    public function setOutputFile($file) {
        $this->output_file = $file;
        return $this;
    }
    
    // Add methods for all major options from the mysqldump command
    
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