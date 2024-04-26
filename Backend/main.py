import os
import time
from azure.cognitiveservices.vision.computervision import ComputerVisionClient 
from azure.cognitiveservices.vision.computervision.models import OperationStatusCodes
from msrest.authentication import CognitiveServicesCredentials
from cv2 import *
import cv2 as cv2
import io
import sys


# Read the environment variables using os.environ
subscription_key = 'aefa583bb71744d18af837fcd01525a6'
endpoint = 'https://licenseplaterecognition.cognitiveservices.azure.com/'
credentials = CognitiveServicesCredentials(subscription_key)
client = ComputerVisionClient(endpoint, credentials)
computervision_client = ComputerVisionClient(endpoint, CognitiveServicesCredentials(subscription_key))

def readLicensePlate(imageName):
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
    print("End of Computer Vision quickstart.")
    if read_result.status == OperationStatusCodes.succeeded:
        for text_result in read_result.analyze_result.read_results:
            for line in text_result.lines:
                if any(char.isdigit() for char in line.text) and any(char.isalpha() for char in line.text):
                    # Return the license plate number if found
                    return line.text

    # Return None if no license plate number is found
    return None

# Main code here
if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python main.py <image_path>")
        sys.exit(1)
    
    image_path = sys.argv[1]
    license_plate_number = readLicensePlate(image_path)
    if license_plate_number is not None:
        print(license_plate_number)  # Print the result to the console
    else:
        print("No license plate number found")