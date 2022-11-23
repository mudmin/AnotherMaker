import gpiod
import sys
import time

leds = [6, 5, 13, 11, 16, 19, 9, 12, 26]
gpiod_pins = [] # gpiod objects

chip = gpiod.chip(0) # 0 chip 

for pin in leds:
    gpiod_pins.append(chip.get_line(pin))
    config = gpiod.line_request()
    config.consumer = "Blink"
    config.request_type = gpiod.line_request.DIRECTION_OUTPUT
    gpiod_pins[-1].request(config)


while(True):
    for pin in gpiod_pins:
        pin.set_value(1)
        time.sleep(0.1)
    for pin in gpiod_pins:
        pin.set_value(0)
        time.sleep(0.05)