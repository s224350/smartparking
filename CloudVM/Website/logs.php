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
    <title>Logs - EzPark</title>
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
                    <li class="nav-item"><a class="nav-link" href="parkingPermits.php">Parking Permits</a></li>
                    <li class="nav-item"><a class="nav-link active" href="logs.php">Logs</a></li>
                    <li class="nav-item"><a class="nav-link" href="logout.php">Logout</a></li>
                </ul>
            </div>
        </div>
    </nav>
    <main class="page project-page">
        <section class="portfolio-block project">
            <div class="container">
                <div class="heading">
                    <h2>Logs</h2>
                </div>
                <div class="row">
                    <div class="col"></div>
                    <div class="col-lg-8 col-xl-6 col-xxl-6">
                        <div class="table-responsive">
                            <table class="table">
                                <thead>
                                    <tr>
                                        <th>Timestamp</th>
                                        <th>Employee</th>
                                        <th>License Plate</th>
                                        <th>Action</th>
                                    </tr>
                                </thead>
                                <tbody>
                                    <!-- PHP code to fetch logs from database -->
                                    <tr>
                                        <td>2023-10-14T22:11:20</td>
                                        <td>Jens</td>
                                        <td>XYZ00</td>
                                        <td>Left</td>
                                    </tr>
                                    <tr>
                                        <td>2023-10-14T22:11:20</td>
                                        <td>Per</td>
                                        <td>ABC11</td>
                                        <td>Parked</td>
                                    </tr>
                                    <tr>
                                        <td>2023-10-14T22:11:20</td>
                                        <td>Unknown</td>
                                        <td>ABC11</td>
                                        <td>Invalid license plate</td>
                                    </tr>
                                </tbody>
                            </table>
                        </div>
                    </div>
                    <div class="col"></div>
                </div>
            </div>
        </section>
    </main>
    <script src="assets/bootstrap/js/bootstrap.min.js"></script>
    <script src="assets/js/pikaday.min.js"></script>
    <script src="assets/js/theme.js"></script>
</body>

</html>