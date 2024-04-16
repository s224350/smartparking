<?php
session_start();
print isset($_SESSION["loggedIn"]);
print "|";
print $_SESSION["loginUser"];
?>
