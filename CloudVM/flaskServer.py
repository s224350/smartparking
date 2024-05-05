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

mydb = mysql.connector.connect(
  host="localhost",
  user="phpuser",
  password="TocMiciVv3j4edc!",
  database="parkingProject"
)

app = Flask(__name__)
app.config['SOCK_SERVER_OPTIONS'] = {'ping_interval': 25}
sock = Sock(app)

sockets = {}

nonParkedCars = []




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

def convertParkingLotStateToBinary(parkingLotState):
    outputString = ""
    for spot in parkingLotState:
        if spot['occupied']:
            outputString += "1"
        else:
            outputString += "0"
    return outputString
    


def getPermitIDbyPlate(licensePlate):
    SQLquery = "SELECT id FROM parkingPermits WHERE licensePlate='"+licensePlate+"'"
    with mydb.cursor() as cursor:
        cursor.execute(SQLquery)
        result = cursor.fetchall()
        if len(result) > 0:
            return result[0][0]
    return 0

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

def sendUpdateToAllSockets(data):
    print("Sending update")
    for socket in sockets:
        sockets[socket].send(json.dumps(data))




@sock.route('/WS/socket')
def wssocket(ws):
    print("Someone connected to the echo route")
    connectionId = str(uuid.uuid4())
    sockets[connectionId] = ws
    print("Connection ID: "+connectionId)

    print("Sending initial data")
    ws.send(json.dumps(getParkingLotState()))
    
    while True:
        try:
            data = ws.receive()
            if data == "ping":
                ws.send("pong")
        except Exception as e:
            sockets.pop(connectionId)
            print("Connection closed: "+connectionId)




@app.route("/DUE/upload", methods=["POST"])
def process_image():
    print("#upload#")
    print("Received image of license plate")
    file = request.files['file_post']
    id = str(uuid.uuid4())+".jpg"
    file.save(id)
    print("Image saved as "+id)

    licensePlate = licensePlateOCR.readLicensePlate(id)
    if len(licensePlate) != 5:
        print("Invalid license plate")
        return "0"

    permitID = getPermitIDbyPlate(licensePlate)
    if permitID == 0:
        print("License plate " + licensePlate + " is not registered in permit database")
        return "0"
    
    print("Car with license plate:"+licensePlate+" and permitID:"+str(permitID)+" has entered the parking lot")
    nonParkedCars.append(permitID)
    return "1"


@app.route('/DUE/carLeft', methods=["POST"])
def carLeft():
    print("#carLeft#")
    if len(nonParkedCars) == 0:
        print("ERROR - No cars to leave")
        return ""
    print("Car with permitID:"+str(nonParkedCars[0])+" left the parking lot")
    nonParkedCars.pop(0)


@app.route('/DUE/update', methods=["POST"])
def update():

    


    receivedData = request.data
    if len(receivedData) != 1:
        return "Invalid data"
    
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
    print(currentSpotStatus)
    print(nonParkedCars)

    sendUpdateToAllSockets(currentSpotStatus)

    return convertParkingLotStateToBinary(currentSpotStatus)



@app.route('/DUE/spotUpdate', methods=["POST"])
def helloPOST():
    print(request.headers)
    print(request.get_data())
    print(request.data)
    return 'success'



if __name__ == "__main__":
    handler = RotatingFileHandler('app.log', maxBytes=100000, backupCount=3)
    logger = logging.getLogger('tdm')
    logger.setLevel(logging.ERROR)
    logger.addHandler(handler)
    app.run(debug=True,port=6123)