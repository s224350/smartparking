<?php
   include("config.php");
   session_start();
   $error='';
   if($_SERVER["REQUEST_METHOD"] == "POST") {
   
      $username = mysqli_real_escape_string($db,$_POST['username']);
      $password = mysqli_real_escape_string($db,$_POST['password']); 

      $sql = "SELECT * FROM users WHERE username = '$username' and password = '$password'";

      $result = mysqli_query($db,$sql);      
      $row = mysqli_num_rows($result);      
      $count = mysqli_num_rows($result);

      if($count == 1) {
         $_SESSION['loggedIn'] = true;
         $_SESSION['loginUser'] = $username;
         header("location: overview.php");
      } else {
         $error = "Your Login Name or Password is invalid";
      }
   }

    if(isset($_SESSION["loggedIn"]) && $_SESSION["loggedIn"] == true){
        header("location: overview.php");
        exit;
    }

?>

<!DOCTYPE html>
<html data-bs-theme="light">

<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, shrink-to-fit=no">
    <title>Login - EzPark</title>
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
                    <li class="nav-item"><a class="nav-link active" href="login.php">Login</a></li>
                </ul>
            </div>
        </div>
    </nav>
    <main class="page contact-page">
        <section class="portfolio-block contact">
            <div class="container">
                <div class="heading">
                    <h2>Login</h2>
                </div>

                
                

                <form class="shadow-lg" data-bs-theme="light" style="max-width: 450px;" method="post" action="login.php">
                    

                    <?php if ($error != '') { ?>
                        <div class="alert alert-danger" role="alert"><?php echo $error; ?></div>
                    <?php } ?>

                    

                    <div class="mb-3"><label class="form-label" for="name">Username</label><input class="form-control item" type="text" id="name" name="username"></div>
                    <div class="mb-3"><label class="form-label" for="subject">Password</label><input class="form-control item" type="text" id="subject" name="password"></div>
                    <div class="mb-3"></div>
                    <div class="mb-3"></div>
                    <div class="mb-3 mt-4"><button class="btn btn-primary btn-lg d-block w-100" type="submit">Login</button></div>
                </form>
            </div>
        </section>
    </main>
    <script src="assets/bootstrap/js/bootstrap.min.js"></script>
    <script src="assets/js/pikaday.min.js"></script>
    <script src="assets/js/theme.js"></script>
</body>

</html>