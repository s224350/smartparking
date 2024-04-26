<!DOCTYPE html>
<html>

<body>

<h1>34346 Smartparking</h1>

<?php
// Define the path to the image file
$image_path = "/img/Alf.jpg";

// Execute the Python script with the image file path as an argument
$output = exec("python main.py " . escapeshellarg($image_path));
echo $output; // Output any result from the Python script
?>


</body>
</html>
