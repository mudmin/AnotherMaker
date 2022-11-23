import RPi.GPIO as GPIO
import time

# You can reverse the array in the program too!
groupsDown = [
    [6],# top
    [5, 13], # 2nd row
    [11, 16, 19], # 3rd 
    [9, 26], # 4th
    [12] # bottom
    ]

groupsUp = [
    [12], # bottom
    [9, 26], # 4th
    [11, 16, 19], # 3rd 
    [5, 13], # 2nd row
    [6]# top
    ]

# assign which group used in code
groups = groupsUp

GPIO.setmode(GPIO.BCM) 

for pins in groups:
    for pin in pins:
        GPIO.setup(pin, GPIO.OUT)

while(True):
    for pins in groups:
        for pin in pins:
            GPIO.output(pin, GPIO.HIGH)
        time.sleep(0.1)

    for pins in groups:
        for pin in pins:
            GPIO.output(pin, GPIO.LOW)
        time.sleep(0.2)

GPIO.cleanup()