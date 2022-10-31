# Raspberry Pi Christmas tree.
Raspberry pi christmas tree for Programming Beginners.

You can get PCB(or gerber files) at https://www.pcbway.com/project/shareproject/Raspberry_Pi_Christmas_tree.html

I have some extra pcb.. If you are picking it up at Downtown Toronto area in person, I will give it to you. I am too lazy to ship it ;-)
Contact me twitter @KotaMorishita

![lib directory contents](/board.jpg)

Each LEDs are directly connected to GPIO 6, 5, 13, 11, 16, 19, 9, 12, 26

# Set up cron for Christmas all the way
```bash
sudo crontab -e
```

Add one line at the end of file.(Adjust the path)
```bash
@reboot python3 /home/pi/rpi-christmas-tree/example/sequence.py 
```
