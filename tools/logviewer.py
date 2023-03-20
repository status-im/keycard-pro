from PIL import Image, ImageTk 
from struct import unpack
import tkinter as tk 
import serial

IMAGE_SIZE = (480, 480)
FRAME_SIZE = IMAGE_SIZE[0] * IMAGE_SIZE[1]
BAUD_RATE = 1176470
PORT_NAME = 'COM4'
START_MARKER = b'\xa5\xca\xfe\x5a'

tkimage = None
ser = None

def seek_start():
    found = 0
    while found < 4:
        marker = ser.read(1)
        if len(marker) == 0:
            return False
        if marker[0] == START_MARKER[found]:
            found = found + 1
        else:
            found = 0
    return True

def read_block(dlen):
    data = bytearray(dlen)
    v = memoryview(data)
    read = 0
    while(read < dlen):
        read = read + ser.readinto(v[read:])
    return data

def poll_serial():
    try:
        if seek_start():
            header = read_block(5)
            (msgtype, dlen) = unpack('<bI', header)
            data = read_block(dlen)

            if msgtype == 0:
                print(data.decode('ascii'))
            elif msgtype == 1:
                im = Image.frombytes('L', IMAGE_SIZE, bytes(data))
                tkimage.paste(im)
            elif msgtype == 2:
                print(data)
    except:
        print("Transmission error")
    root.after(1, poll_serial)

root = tk.Tk()
tkimage = ImageTk.PhotoImage("L", IMAGE_SIZE)
tk.Label(root, image=tkimage).pack()
ser = serial.Serial(PORT_NAME, BAUD_RATE, timeout=0)
poll_serial()
root.mainloop()
