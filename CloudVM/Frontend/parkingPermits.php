<?php
session_start();
if(!isset($_SESSION["loggedIn"]) || $_SESSION["loggedIn"] !== true){
    header("location: login.php");
    exit;
}

require("handleParkingPermitPOST.php");
?>


<!DOCTYPE html>
<html data-bs-theme="light">

<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, shrink-to-fit=no">
    <title>Parking Permits - EzPark</title>
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
                    <li class="nav-item"><li class="nav-item"><a class="nav-link" href="parkingPermits.php">Parking Permits</a></li>
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
                    <h2>Parking Permits</h2>
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


                                    <?php
                                        require("config.php");
                                        $sqlQuery = "SELECT * FROM parkingPermits";
                                        $result = mysqli_query($db,$sqlQuery);  

                                        if ($result->num_rows > 0) {
                                            while($row = $result->fetch_assoc()) {
                                                echo '<tr>';
                                                echo '<input type="hidden" value="'.$row["id"].'">';
                                                echo '<td><input class="form-control" type="text" value="'.$row["Name"].'"></td>';
                                                echo '<td><input class="form-control" type="text" value="'.$row["licensePlate"].'"></td>';
                                                echo '<td>
                                                    <button class="btn btn-danger item-remove" type="button"><span style="color: rgb(255, 255, 255);">Remove</span></button>
                                                    <button class="btn btn-success item-save" type="button"><span style="color: rgb(255, 255, 255);">Save</span></button>
                                                    </td>
                                                </tr>';
                                            }
                                         }
                                         mysqli_free_result($result);
                                         $db->close();

                                    ?>
                                </tbody>
                            </table>
                        </div><button class="btn btn-success item-newCar" type="button"><span style="color: rgb(255, 255, 255);">Create new parking permit entry</span></button>
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
        function addPermit(e){
            var jsonData ={action:"addPermit"}
            doAction(jsonData);
            console.log("Adding new car");
        }

        function removePermit(e){
            var siblings = e.currentTarget.parentElement.parentElement.children;
            var id = siblings[0].value;
            var jsonData ={action:"removePermit",id:id}
            doAction(jsonData);
            console.log("Removing: " + id);
        }

        function modifyPermit(e){
            var siblings = e.currentTarget.parentElement.parentElement.children;
            var id = siblings[0].value;
            var name = siblings[1].getElementsByTagName("input")[0].value;
            var plate = siblings[2].getElementsByTagName("input")[0].value;
            var jsonData ={action:"modifyPermit",id:id,name:name,plate:plate}
            doAction(jsonData);
            console.log("Saveing: " + id + " " + name + " " + plate);
        }

        function doAction(jsonData){
            fetch("parkingPermits.php", {
                method: "POST",
                body: JSON.stringify(jsonData),
                headers: {
                "Content-type": "application/json; charset=UTF-8"
                }
            }).then(() => {
                window.location.reload();
            });
        }

        document.getElementsByClassName("item-newCar")[0].addEventListener("click", addPermit);

        var elements = document.getElementsByClassName("item-remove");
        for (var i = 0; i < elements.length; i++) {
            elements[i].addEventListener("click", removePermit);
        }

        var elements = document.getElementsByClassName("item-save");
        for (var i = 0; i < elements.length; i++) {
            elements[i].addEventListener("click", modifyPermit);
        }
    </script>
</body>

</html>