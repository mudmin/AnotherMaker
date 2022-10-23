# -*- coding: utf-8 -*-
# sudo -H pip3 install --upgrade luma.oled
# sudo -H pip3 install serial
# 128 x 64
from luma.core.interface.serial import i2c
from luma.core.render import canvas
from luma.oled.device import ssd1306
from PIL import Image, ImageFont, ImageDraw, ImageOps
import time
import os

class ssd1306_oled(object):
    def __init__(self):
        print("oled initialize")

    def setup(self, i2c_address=0x3C):
        self.serial = i2c(port=1, address=i2c_address)
        self.device = ssd1306(self.serial)

        # font configuration
        self.ttf = '/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf'
        basedir = os.path.dirname(os.path.realpath(__file__))
        self.imagedir = os.path.join(basedir, 'images')
        self.font8 = ImageFont.truetype(self.ttf, 8)
        self.font16 = ImageFont.truetype(self.ttf, 16)
        self.font32 = ImageFont.truetype(self.ttf, 32)

        with canvas(self.device) as drawUpdate:
            drawUpdate.text((5, 0), "Hello" , font=self.font32, fill=100)

        time.sleep(10)

monitor = ssd1306_oled()
monitor.setup()