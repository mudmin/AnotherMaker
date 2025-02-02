# pip install keyboard pywin32
# pip install keyboard keyboard

# lazy frehd import scripy by AnotherMaker
# Description: This script is used to import files into the TRS-80 emulator. It takes a directory as an argument and types out the import command for each file in the directory.
# Usage: have trs80gp running with your frhed enabled.
# inside your frehd folder on the pc, make a folder that you want to put all your imports in.  Copy all files there.
# run this script with the folder as the argument.  It will type out the import command for each file.
# example for starting trs80gp

# trs80gp -m1 -frehd_patch -frehd_dir h:\trs80gp\frehd
# then I put a folder inside h:\trs80gp\frehd\import
# python import.py h:\trs80gp\frehd\import

# python import.py h:\trs80gp\frehd\import 2 
# will import to drive 2

# you then have 5 seconds to click on the trs80gp window and then it will start typing out the import commands.
# to cancel the script, hit ctrl c in the terminal window.

import os
import sys
import time
import keyboard  
import win32api, win32con 

if len(sys.argv) not in [2, 3]:
    print("Usage: trsimport.py <directory> [drive_number]")
    print("Example: trsimport.py h:\\trs80gp\\frehd\\import 2")
    sys.exit(1)

directory = sys.argv[1]
# Default to drive 0 if no drive number specified
drive_number = sys.argv[2] if len(sys.argv) == 3 else "0"

if not os.path.isdir(directory):
    print("Error: Provided argument is not a valid directory.")
    sys.exit(1)

# Extract the last folder name from the provided directory.
folder_name = os.path.basename(os.path.normpath(directory))

# Get a sorted list of files in the directory.
files = sorted([f for f in os.listdir(directory) if os.path.isfile(os.path.join(directory, f))])

print(f"Files will be imported to drive {drive_number}")
print("You have 5 seconds to focus on the emulator window...")
time.sleep(5)

def write_with_doubles(text):
    """Write text, handling double characters with space-backspace method"""
    prev_char = ''
    for char in text:
        if char == prev_char:
            # Insert space
            keyboard.write(' ', delay=0.1)
            # Use win32api for backspace
            win32api.keybd_event(0x08, 0, 0, 0)  # VK_BACK = 0x08
            time.sleep(0.1)
            win32api.keybd_event(0x08, 0, win32con.KEYEVENTF_KEYUP, 0)
            time.sleep(0.1)
        keyboard.write(char, delay=0.1)
        prev_char = char

for filename in files:
    # Transform "filename.ext" into "filename/ext"
    if '.' in filename:
        idx = filename.find('.')
        transformed = filename[:idx] + '/' + filename[idx+1:]
    else:
        transformed = filename

    # Build the command string using the last folder name and drive number
    command = f"import2 /{folder_name}/{filename} {transformed}:{drive_number}"

    write_with_doubles(command)
    win32api.keybd_event(win32con.VK_RETURN, 0, 0, 0)
    time.sleep(0.1)
    win32api.keybd_event(win32con.VK_RETURN, 0, win32con.KEYEVENTF_KEYUP, 0)
    
    time.sleep(2)