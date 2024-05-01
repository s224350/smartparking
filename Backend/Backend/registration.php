<?php
// SQLite connection
try {
    $db = 'db_Persons.db';
    $pdo = new PDO('sqlite:' . $db);

    // Set error handling to exceptions
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

    // Get records from People table
    $People = $pdo->query('SELECT * FROM People');
    $records = [];
    while ($row = $People->fetch(PDO::FETCH_ASSOC)) {
        $records[] = $row;
    }
} catch (Exception $e) {
    echo $e->getMessage();
    exit();
}

// Function to update People table
function editCar($id, $name, $plate, $pdo) {
    $stmt = $pdo->prepare('UPDATE People SET Name = :name, NumberPlate = :plate WHERE UID = :id');
    $stmt->bindParam(':name', $name, PDO::PARAM_STR);
    $stmt->bindParam(':plate', $plate, PDO::PARAM_STR);
    $stmt->bindParam(':id', $id, PDO::PARAM_INT);
    $stmt->execute();
}

// Function to register a car
function registerCar($pdo) {
    $name='xyz';
    $plate='None';
    $stmt = $pdo->prepare('INSERT INTO People (Name, NumberPlate) VALUES (:name, :plate)');
    $stmt->bindParam(':name', $name, PDO::PARAM_STR);
    $stmt->bindParam(':plate', $plate, PDO::PARAM_STR);
    $stmt->execute();
}

// Function to deregister a car
function deregisterCar($id, $pdo) {
    $stmt = $pdo->prepare('DELETE FROM People WHERE UID = :id');
    $stmt->bindParam(':id', $id, PDO::PARAM_INT);
    $stmt->execute();
    if ($stmt->rowCount() > 0) {
        echo "Car with ID $id deregistered successfully.";
    } else {
        echo "Car with ID $id not found.";
    }
}

// Main code
$jsondata = file_get_contents('php://input');

// Decode the JSON data
$data = json_decode($jsondata, true);

// Check if JSON decoding was successful
if ($data !== null) {
    // Check if action is provided in the JSON data
    if (isset($data['action'])) {
        // Perform actions based on the value of 'action'
        switch ($data['action']) {
            case 'deregisterCar':
                if (isset($data['id'])) {
                    deregisterCar($data['id'], $pdo);
                } else {
                    echo "No ID provided to deregister the car.";
                }
                break;
            case 'registerCar':
                if (isset($data['id']) &&isset($data['name']) && isset($data['plate'])) {
                    editCar($data['id'],$data['name'], $data['plate'], $pdo);
                } else {
                    registerCar($pdo);
                    echo "Name and/or plate not provided to register the car.";
                }
                break;
            default:
                echo "Unknown action.";
        }
    } else {
        echo "No action provided in JSON data.";
    }
} else {
    echo "Failed to decode JSON data.";
    //Test of functions
    editCar(60,'Mustafa','DK77777',$pdo);
    registerCar($pdo);
    editCar(61,'Caroline','DK89898',$pdo);
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
}

// Close database connection
$pdo = null;
?>
