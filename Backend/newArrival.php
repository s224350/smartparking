<?php
try {
    $db = 'db_Persons.db';
    $pdo = new PDO('sqlite:' . $db);
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

    $People = $pdo->query('SELECT * FROM People');
    $records = [];
    while ($row = $People->fetch(PDO::FETCH_ASSOC)) {
        $records[] = $row;
    }
} catch (Exception $e) {
    echo $e->getMessage();
    exit();
}

function findID($input, $records, $pdo) {
    foreach ($records as $row) {
        if ($row['NumberPlate'] == $input) {
            echo $row['Name'] . " has arrived.";
            echo '<br>';
            return $row['UID'];
        }
    }
}

function freeParkingSpots($pdo) {
    $freeSpots = [];
    $Spots = $pdo->query('SELECT * FROM Parking');
    while ($row = $Spots->fetch(PDO::FETCH_ASSOC)) {
        if ($row['Parking'] == 0) {
            $freeSpots[] = $row['UID'];
        }
    }
    return $freeSpots;
}
#Test of code
$output = 'DK12345';
#Test over 
$id = findID($output, $records, $pdo);
echo "ID of the arrived person: $id<br>";
$freeSpots = freeParkingSpots($pdo);
echo "Free parking spots: " . implode(', ', $freeSpots);
?>
