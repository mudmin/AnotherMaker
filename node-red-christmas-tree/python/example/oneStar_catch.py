import RPi.GPIO as GPIO
import time

pin = 6
GPIO.setmode(GPIO.BCM)
GPIO.setup(pin, GPIO.OUT)

try:
  while(True):
    GPIO.output(pin, GPIO.HIGH)
    time.sleep(0.5)
    GPIO.output(pin, GPIO.LOW)
    time.sleep(0.5)
finally:
    GPIO.cleanup()
