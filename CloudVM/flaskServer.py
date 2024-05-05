from flask import Flask, request, jsonify
from flask_sock import Sock
from PIL import Image
import licensePlateOCR
import uuid
import os
import mysql.connector
import json
import logging
from logging.handlers import RotatingFileHandler

# Create mysql connector
mydb = mysql.connector.connect(
  host="localhost",
  user="phpuser",
  password="TocMiciVv3j4edc!",
  database="parkingProject"
)

app = Flask(__name__)
app.config['SOCK_SERVER_OPTIONS'] = {'ping_interval': 25}
sock = Sock(app)

# List of all connected sockets
sockets = {}

# List of cars inside parking lot, but not parked
nonParkedCars = []

# Get parking lot state from database, and return as list of dictionaries with id, activePermit, name, occupied boolean
def getParkingLotState():
    SQLquery = "SELECT parkingSpots.id,parkingSpots.activePermit,parkingPermits.name FROM parkingSpots LEFT JOIN parkingPermits ON parkingPermits.id=parkingSpots.activePermit"
    with mydb.cursor() as cursor:
        cursor.execute(SQLquery)
        myresult = cursor.fetchall()
        spots = []
        for spot in myresult:
            spots.append({"id":spot[0],"activePermit":spot[1],"name":spot[2],"occupied":spot[1] != 0})
        print(spots)
        return spots
    return []

# Convert the data from the getParkingLotState function to a binary string based on the occupied status
def convertParkingLotStateToBinary(parkingLotState):
    outputString = ""
    for spot in parkingLotState:
        if spot['occupied']:
            outputString += "1"
        else:
            outputString += "0"
    return outputString
    
# Get corresponding permitID from license plate from the database
def getPermitIDbyPlate(licensePlate):
    SQLquery = "SELECT id FROM parkingPermits WHERE licensePlate='"+licensePlate+"'"
    with mydb.cursor() as cursor:
        cursor.execute(SQLquery)
        result = cursor.fetchall()
        if len(result) > 0:
            return result[0][0]
    return 0

# Get the permit id currently active for a specific parking spot
def getPermitIDbySpot(spotID):
    SQLquery = "SELECT activePermit FROM parkingSpots WHERE id="+str(spotID)
    with mydb.cursor() as cursor:
        cursor.execute(SQLquery)
        result = cursor.fetchall()
        if len(result) > 0:
            return result[0][0]
    return 0

def registerParkedCar(permitID,spotID):
    SQLquery = "UPDATE parkingSpots SET activePermit="+str(permitID)+" WHERE id="+str(spotID)
    with mydb.cursor() as cursor:
        cursor.execute(SQLquery)
        mydb.commit()

def registerDeparkedCar(spotID):
    SQLquery = "UPDATE parkingSpots SET activePermit=0 WHERE id="+str(spotID)
    with mydb.cursor() as cursor:
        cursor.execute(SQLquery)
        mydb.commit()

# Tell all connected clients the new parking lot state
def sendUpdateToAllSockets(data):
    print("Sending update")
    for socket in sockets:
        sockets[socket].send(json.dumps(data))

# Route for websocket connection
@sock.route('/WS/socket')
def wssocket(ws):
    # Store the socket in the list of connected sockets
    connectionId = str(uuid.uuid4())
    sockets[connectionId] = ws
    print("Connection ID: "+connectionId)

    # Send the current parking lot state to the client
    ws.send(json.dumps(getParkingLotState()))
    
    # Keep the connection alive by responding to pings
    while True:
        try:
            data = ws.receive()
            if data == "ping":
                ws.send("pong")
        except Exception as e:
            sockets.pop(connectionId)
            print("Connection closed: "+connectionId)

# Route when DUE sends an image of a license plate
@app.route("/DUE/upload", methods=["POST"])
def process_image():
    print("#upload#")
    print("Received image of license plate")

    # Save the received image
    file = request.files['file_post']
    id = str(uuid.uuid4())+".jpg"
    file.save(id)
    print("Image saved as "+id)

    # Read the license plate from the image
    licensePlate = licensePlateOCR.readLicensePlate(id)

    # validate the license plate
    if len(licensePlate) != 5:
        print("Invalid license plate")
        return "0"

    # Get the permitID of the car with the license plate
    permitID = getPermitIDbyPlate(licensePlate)
    if permitID == 0:
        print("License plate " + licensePlate + " is not registered in permit database")
        return "0"
    
    print("Car with license plate:"+licensePlate+" and permitID:"+str(permitID)+" has entered the parking lot")
    nonParkedCars.append(permitID)
    return "1"

# Route when DUE sends a carLeft request, which means that a car has left the parking lot 
@app.route('/DUE/carLeft', methods=["POST"])
def carLeft():
    print("#carLeft#")
    if len(nonParkedCars) == 0:
        print("ERROR - No cars to leave")
        return ""
    print("Car with permitID:"+str(nonParkedCars[0])+" left the parking lot")
    nonParkedCars.pop(0)

# Route when DUE sends a update request, which means that a car has parked or left a parking spot
# The first 7 bits of the received byte are the spotID, and the last bit is the occupied status
@app.route('/DUE/update', methods=["POST"])
def update():
    receivedData = request.data

    # Exit if the received data is not 1 byte
    if len(receivedData) != 1:
        return "Invalid data"
    
    # Parse byte
    receivedByte = receivedData[0]
    spotID = ((receivedByte >> 1) & 0b1111111) + 1
    occupied = receivedByte & 0b1

    if occupied:
        if len(nonParkedCars) == 0:
            print("ERROR - No car to park")
        else:
            registerParkedCar(nonParkedCars[0],spotID)
            nonParkedCars.pop(0)
    else:
        permitID = getPermitIDbySpot(spotID)
        if permitID == 0:
            print("ERROR - No car parked in spot "+str(spotID))
            return ""
        registerDeparkedCar(spotID)
        nonParkedCars.append(permitID)

    currentSpotStatus = getParkingLotState()
    print("#update#")
    print("Car has " + ("parked" if str(occupied) else "left") + " spot "+str(spotID))

    # Update all connected clients with the new parking lot state
    sendUpdateToAllSockets(currentSpotStatus)

    # Return the new parking lot state as binary string
    return convertParkingLotStateToBinary(currentSpotStatus)

if __name__ == "__main__":
    handler = RotatingFileHandler('app.log', maxBytes=100000, backupCount=3)
    logger = logging.getLogger('tdm')
    logger.setLevel(logging.ERROR)
    logger.addHandler(handler)
    app.run(debug=True,port=6123)