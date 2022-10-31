import RPi.GPIO as GPIO
import time

leds = [6, 5, 13, 11, 16, 19, 9, 12, 26] 

GPIO.setmode(GPIO.BCM) 

for pin in leds:
    GPIO.setup(pin, GPIO.OUT)

while(True):
    for pin in leds:
        GPIO.output(pin, GPIO.HIGH)
        time.sleep(0.1)
    for pin in leds:
        GPIO.output(pin, GPIO.LOW)
        time.sleep(0.05)

GPIO.cleanup()