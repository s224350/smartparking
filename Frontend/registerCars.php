<?php
session_start();
if(!isset($_SESSION["loggedIn"]) || $_SESSION["loggedIn"] !== true){
    header("location: login.php");
    exit;
}
?>


<!DOCTYPE html>
<html data-bs-theme="light">

<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, shrink-to-fit=no">
    <title>Project Page - EzPark</title>
    <link rel="stylesheet" href="assets/bootstrap/css/bootstrap.min.css">
    <link rel="stylesheet" href="assets/css/Alexandria.css">
    <link rel="stylesheet" href="assets/css/Lato.css">
    <link rel="stylesheet" href="assets/css/pikaday.min.css">
</head>

<body>
    <nav class="navbar navbar-expand-lg fixed-top portfolio-navbar gradient navbar-dark">
        <div class="container"><a class="navbar-brand logo" href="#" style="font-family: Alexandria, sans-serif;">EzPark</a><button data-bs-toggle="collapse" class="navbar-toggler" data-bs-target="#navbarNav"><span class="visually-hidden">Toggle navigation</span><span class="navbar-toggler-icon"></span></button>
            <div class="collapse navbar-collapse" id="navbarNav">
                <ul class="navbar-nav ms-auto">
                    <li class="nav-item"><a class="nav-link" href="overview.php">Overview</a></li>
                    <li class="nav-item"><a class="nav-link active" href="registerCars.php">Register Cars</a></li>
                    <li class="nav-item"><a class="nav-link" href="logs.php">Logs</a></li>
                    <li class="nav-item"><a class="nav-link" href="logout.php">Logout</a></li>
                </ul>
            </div>
        </div>
    </nav>
    <main class="page project-page">
        <section class="portfolio-block project">
            <div class="container">
                <div class="heading">
                    <h2>Register cars</h2>
                </div>
                <div class="row">
                    <div class="col"></div>
                    <div class="col-lg-8 col-xl-6 col-xxl-6">
                        <div class="table-responsive">
                            <table class="table">
                                <thead>
                                    <tr>
                                        <th>Name</th>
                                        <th>License Plate</th>
                                        <th>Action</th>
                                    </tr>
                                </thead>
                                <tbody>
                                    <tr>
                                        <input type="hidden" value="1">
                                        <td><input type="text"></td>
                                        <td><input type="text"></td>
                                        <td>
                                        <button class="btn btn-danger item-remove" type="button"><span style="color: rgb(255, 255, 255);">Remove</span></button>
                                            <button class="btn btn-success item-save" type="button"><span style="color: rgb(255, 255, 255);">Save</span></button>
                                        </td>
                                    </tr>
                                    <tr>
                                        <input type="hidden" value="2">
                                        <td><input type="text"></td>
                                        <td><input type="text"></td>
                                        <td>
                                            <button class="btn btn-danger item-remove" type="button"><span style="color: rgb(255, 255, 255);">Remove</span></button>
                                            <button class="btn btn-success item-save" type="button"><span style="color: rgb(255, 255, 255);">Save</span></button>
                                        </td>
                                    </tr>
                                </tbody>
                            </table>
                        </div><button class="btn btn-success item-newCar" type="button"><span style="color: rgb(255, 255, 255);">Register new car</span></button>
                    </div>
                    <div class="col"></div>
                </div>
            </div>
        </section>
    </main>
    <script src="assets/bootstrap/js/bootstrap.min.js"></script>
    <script src="assets/js/pikaday.min.js"></script>
    <script src="assets/js/theme.js"></script>


    <script>
        function registerSampleCar(e){
            var jsonData ={action:"registerCar"}
            doAction(jsonData);
            console.log("Adding new car");
        }

        function deregisterCar(e){
            var siblings = e.currentTarget.parentElement.parentElement.children;
            var id = siblings[0].value;
            var jsonData ={action:"deregisterCar",id:id}
            doAction(jsonData);
            console.log("Removing: " + id);
        }

        function editRegisteredCar(e){
            var siblings = e.currentTarget.parentElement.parentElement.children;
            var id = siblings[0].value;
            var name = siblings[1].getElementsByTagName("input")[0].value;
            var plate = siblings[2].getElementsByTagName("input")[0].value;
            var jsonData ={action:"deregisterCar",id:id,name:name,plate:plate}
            doAction(jsonData);
            console.log("Saveing: " + id + " " + name + " " + plate);
        }

        function doAction(jsonData){
            fetch("registration.php", {
                method: "POST",
                body: JSON.stringify(jsonData),
                headers: {
                "Content-type": "application/json; charset=UTF-8"
                }
            });
        }

        document.getElementsByClassName("item-newCar")[0].addEventListener("click", registerSampleCar);
        var elements = document.getElementsByClassName("item-remove");
        for (var i = 0; i < elements.length; i++) {
        elements[i].addEventListener("click", deregisterCar);
        }
        var elements = document.getElementsByClassName("item-save");
        for (var i = 0; i < elements.length; i++) {
        elements[i].addEventListener("click", editRegisteredCar);
        }
    </script>
</body>

</html>