import os
import time
from azure.cognitiveservices.vision.computervision import ComputerVisionClient
from azure.cognitiveservices.vision.computervision.models import OperationStatusCodes
from msrest.authentication import CognitiveServicesCredentials
from cv2 import *
import cv2 as cv2
import io
import re

# Read the environment variables using os.environ
subscription_key = 'aefa583bb71744d18af837fcd01525a6'
endpoint = 'https://licenseplaterecognition.cognitiveservices.azure.com/'
credentials = CognitiveServicesCredentials(subscription_key)
client = ComputerVisionClient(endpoint, credentials)
computervision_client = ComputerVisionClient(endpoint, CognitiveServicesCredentials(subscription_key))

def readLicensePlate(imageName):
    print("Sending image to azure for OCR")
    # read the image with OpenCV
    image = cv2.imread(imageName)
    retval, buffer = cv2.imencode('.jpg', image)
    jpeg_bytes = buffer.tobytes()
    image_stream = io.BytesIO(jpeg_bytes)

    read_response = computervision_client.read_in_stream(image_stream, raw=True)
    read_operation_location = read_response.headers["Operation-Location"]
    operation_id = read_operation_location.split("/")[-1]
    while True:
        read_result = computervision_client.get_read_result(operation_id)
        if read_result.status not in ['notStarted', 'running']:
            break
        time.sleep(1)
    if read_result.status == OperationStatusCodes.succeeded:
        print("Receving OCR results from azure")
        for text_result in read_result.analyze_result.read_results:
            for line in text_result.lines:
                # Check if the line contains a license plate number
                print("Detected text:", line.text)
                plate = re.findall("([A-Z]{2} ?[0-9]{3})", line.text)
                if plate:
                    plate = plate[0].replace(" ","")
                    print("License plate number:", plate)
                    return plate
    return ""
