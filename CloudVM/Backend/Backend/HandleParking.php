<?php
include 'newArrival.php';

try {
    $db = 'db_Persons.db';
    $pdo = new PDO('sqlite:' . $db);
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

    $Parking = $pdo->query('SELECT * FROM Parking');
    $records = [];
    while ($row = $Parking->fetch(PDO::FETCH_ASSOC)) {
        $records[] = $row;
    }
} catch (Exception $e) {
    echo $e->getMessage();
    exit();
}

function occupiedSpot($id, $pdo) {
    $stmt = $pdo->prepare('UPDATE Parking SET Parking=1 WHERE UID=:id');
    $stmt->bindValue(':id', $id, PDO::PARAM_INT);
    $stmt->execute();
}

function availableSpot($id, $pdo) {
    $stmt = $pdo->prepare('UPDATE Parking SET Parking=0 WHERE UID=:id');
    $stmt->bindValue(':id', $id, PDO::PARAM_INT);
    $stmt->execute();
}

#Test of code
occupiedSpot(3, $pdo);
$freeSpots = freeParkingSpots($pdo);
echo "<br>Free parking spots: " . implode(', ', $freeSpots);
availableSpot(3, $pdo);

$freeSpots = freeParkingSpots($pdo);
echo "<br>Free parking spots: " . implode(', ', $freeSpots);
#Test over
$pdo = null; 
?>
