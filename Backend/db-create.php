<?php
// SQLite database file path
$database_file = 'db_Persons.db';

try {
    // Connect to SQLite database using PDO
    $pdo = new PDO('sqlite:' . $database_file);

    // Set error handling to exceptions
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

    // Create People table
    $pdo->exec('CREATE TABLE People (
        UID INTEGER PRIMARY KEY AUTOINCREMENT,
        Name TEXT,
        NumberPlate TEXT
    )');

    // Create Parking table
    $pdo->exec('CREATE TABLE Parking (
        UID INTEGER PRIMARY KEY AUTOINCREMENT,
        Parking INTEGER
    )');
    
    // Close database connection
    $pdo = null;
} catch (PDOException $e) {
    // Handle database connection errors
    echo "Connection failed: " . $e->getMessage();
}

?>
