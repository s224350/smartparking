<?php
session_start();
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

<body style="overflow: hidden;">
    <nav class="navbar navbar-expand-lg fixed-top portfolio-navbar gradient navbar-dark">
        <div class="container"><a class="navbar-brand logo" href="#" style="font-family: Alexandria, sans-serif;">EzPark</a><button data-bs-toggle="collapse" class="navbar-toggler" data-bs-target="#navbarNav"><span class="visually-hidden">Toggle navigation</span><span class="navbar-toggler-icon"></span></button>
            <div class="collapse navbar-collapse" id="navbarNav">
                <ul class="navbar-nav ms-auto">
                    <li class="nav-item"><a class="nav-link active" href="overview.php">Overview</a></li>
                    <?php
                    if (isset($_SESSION["loggedIn"]) && $_SESSION["loggedIn"] === true) {
                        echo '<li class="nav-item"><a class="nav-link" href="registerCars.php">Register Cars</a></li>';
                        echo '<li class="nav-item"><a class="nav-link" href="logs.php">Logs</a></li>';
                        echo '<li class="nav-item"><a class="nav-link" href="logout.php">Logout</a></li>';
                    } else {
                        echo '<li class="nav-item"><a class="nav-link" href="login.php">Login</a></li>';
                    }
                    ?>
                </ul>
            </div>
        </div>
    </nav>

    <canvas id="canvas" style="background-color: #666666;"></canvas>

    <script src="assets/bootstrap/js/bootstrap.min.js"></script>
    <script src="assets/js/pikaday.min.js"></script>
    <script src="assets/js/theme.js"></script>
    <script>
        var canvas = document.getElementById('canvas');

        const spaceWidth = 100;
        const spaceHeight = 200;
        const parkingSpacesCount = 4;
        const startX = -(parkingSpacesCount/2)*spaceWidth;
        const startY = -spaceHeight;

        const carsImg = [new Image(),new Image(),new Image()];
        const cars = [];
        const parkingSpaces = [];


        function resizeCanvas(){
            const ctx = document.getElementById("canvas").getContext("2d");
            canvas.width = window.innerWidth;
            canvas.height = window.innerHeight;
            ctx.translate(canvas.width/2,canvas.height/2);
        }
        resizeCanvas();
        window.addEventListener('resize', resizeCanvas, false);




        function addCar(spotID,carType){
            spot = getParkingSpot(spotID);
            cars.push(new car(spot.x,spot.y,"parked",spotID,carType)); 
        }

        function removeCar(spotID){
            removeCarByParkingSpot(spotID);
        }

        function car(x, y, state, parkingSpotID,type) {
            this.x = x;
            this.y = y;
            this.state = state;
            this.parkingSpotID = parkingSpotID;
            this.type = type;
        }

        function parkingSpace(id, x, y , occupied) {
            this.id = id;
            this.x = x;
            this.y = y;
            this.occupied = occupied;
        }

        function init() {
            carsImg[0].src = "./assets/img/Car1.png";
            carsImg[1].src = "./assets/img/Car2.png";
            carsImg[2].src = "./assets/img/Car3.png";
            
            parkingSpaces.push(new parkingSpace(0,-150,-100,false)); 
            parkingSpaces.push(new parkingSpace(1,-50,-100,false)); 
            parkingSpaces.push(new parkingSpace(2,50,-100,false)); 
            parkingSpaces.push(new parkingSpace(3,150,-100,false)); 

            window.requestAnimationFrame(draw);
        }

        function draw() {
            const ctx = document.getElementById("canvas").getContext("2d");



            /*for (var i=0;i<cars.length;i++){
                if (cars[i].state == "parking"){
                    parkingSpot = getParkingSpot(cars[i].parkingSpotID);
                    console.log(cars[i].x);
                    if (cars[i].x < parkingSpot.x){
                        cars[i].x++;
                    } else if (cars[i].rot > 0){
                        cars[i].rot -= 1;
                    } else if (cars[i].y > parkingSpot.y){
                        cars[i].y--;
                    }
                }
            }*/



            drawParkingLot(ctx);
            drawCars(ctx);

            window.requestAnimationFrame(draw);
        }

        function drawParkingLot(ctx){
            ctx.strokeStyle = 'white';
            ctx.lineWidth = 5;
            ctx.beginPath();

            //Draw horizontal line
            ctx.moveTo(startX, startY);
            ctx.lineTo(startX+parkingSpacesCount*spaceWidth, startY);

            //Draw vertical line
            for (let i = 0; i < parkingSpacesCount+1; i++) {
                ctx.moveTo(startX+i*spaceWidth, startY);
                ctx.lineTo(startX+i*spaceWidth, startY+spaceHeight);
            }
            ctx.stroke();
        }

        function getParkingSpot(id){
            for (var i=0;i<parkingSpaces.length;i++){
                if (parkingSpaces[i].id == id){
                    return {x: parkingSpaces[i].x, y: parkingSpaces[i].y, rot: parkingSpaces[i].rot};
                }
            }
            return null;
        }

        function removeCarByParkingSpot(id){
            for (var i=0;i<cars.length;i++){
                if (cars[i].parkingSpotID == id){
                    cars.splice(i,1);
                }
            }
        }

        function drawCars(ctx){
            for (var i=0;i<cars.length;i++){
                if (cars[i].state != "empty"){
                    drawCar(ctx,cars[i].type,cars[i].x,cars[i].y,0);
                    console.log("Drawing car:"+cars[i].x+","+cars[i].y+",0")
                }
            }
        }

        function drawCar(ctx,type,x,y,angle){
            ctx.save();
            ctx.translate(x, y);
            ctx.rotate(angle*Math.PI/180);
            ctx.drawImage(carsImg[type], -39,-70,78,140);
            ctx.restore();
        }

        init();




</script>


</body>

</html>