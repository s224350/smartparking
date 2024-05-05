<?php
// SQLite database file path
$database_file = 'db_Persons.db';

try {
    // Connect to SQLite database using PDO
    $pdo = new PDO('sqlite:' . $database_file);

    // Set error handling to exceptions
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

    // Function to set data
    function setData($pdo) {
        $pdo->exec("INSERT INTO People (Name, NumberPlate) VALUES 
            ('Alfred', 'DK12345'),
            ('Bent', 'DK45932'),
            ('Charlotte', 'DK84975'),
            ('Dorte', 'DK98347')");
        
        $pdo->exec("INSERT INTO Parking (Parking) VALUES 
            (0), (0), (0), (0)");
    }

    // Call the setData function
    setData($pdo);

    // Query to select data from People table
    $persons = $pdo->query("SELECT * FROM People");
    $records = $persons->fetchAll(PDO::FETCH_ASSOC);
    echo "Total rows are: " . count($records) . "<br>";
    echo "Printing each row<br>";
    foreach ($records as $row) {
        echo "Id: " . $row['UID'] . "<br>";
        echo "Name: " . $row['Name'] . "<br>";
        echo "Licence Plate: " . $row['NumberPlate'] . "<br>";
    }
    echo "<br>";

    // Query to select data from Parking table
    $parking = $pdo->query("SELECT * FROM Parking");
    $p = $parking->fetchAll(PDO::FETCH_ASSOC);
    echo "Parking Spaces: " . count($p) . "<br>";
    foreach ($p as $row) {
        echo "Id: " . $row['UID'] . "<br>";
        echo "Parking: " . $row['Parking'] . "<br>";
    }

    // Close database connection
    $pdo = null;
} catch (PDOException $e) {
    // Handle database connection errors
    echo "Connection failed: " . $e->getMessage();
}
?>
