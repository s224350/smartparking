<?php
// SQLite database file path
$database_file = 'db_Persons.db';

try {
    // Connect to SQLite database using PDO
    $pdo = new PDO('sqlite:' . $database_file);

    // Set error handling to exceptions
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

    // Function to clear data from tables
    function clearTables($pdo) {
        $pdo->exec("DROP TABLE People");
        $pdo->exec("DROP TABLE Parking");
    }

    // Call the clearTables function
    clearTables($pdo);

    // Close database connection
    $pdo = null;

    echo "Tables cleared successfully.";
} catch (PDOException $e) {
    // Handle database connection errors
    echo "Connection failed: " . $e->getMessage();
}
?>
