import sys, getopt
from luma.core.interface.serial import i2c
from luma.core.render import canvas
from luma.oled.device import ssd1306
from PIL import ImageFont

selection = 1

try:
    opts, args = getopt.getopt(sys.argv[1:], "s:", ["selection="])
    for opt, arg in opts:
        if opt in ("-s", "--selection"):
            selection = int(arg)
except:
    pass

try:
    serial = i2c(port=1, address=0x3C)
    device = ssd1306(serial)
    device.cleanup = lambda: None
except:
    sys.exit()

with canvas(device) as draw:
    draw.line((64, 0, 64, 64), fill="white")
    draw.line((0, 32, 128, 32), fill="white")
    
    try:
        font = ImageFont.load_default()
    except:
        font = None 
    
    draw.text((15, 10), "Easy", font=font, fill="white")
    draw.text((80, 10), "Med", font=font, fill="white")
    draw.text((15, 42), "Hard", font=font, fill="white")
    draw.text((80, 42), "Extr", font=font, fill="white")

    box = None
    if selection == 1:   box = [0, 0, 63, 31]
    elif selection == 2: box = [64, 0, 127, 31]
    elif selection == 3: box = [0, 32, 63, 63]
    elif selection == 4: box = [64, 32, 127, 63]
        
    if box:
        draw.rectangle(box, outline="white")
        draw.rectangle([box[0]+1, box[1]+1, box[2]-1, box[3]-1], outline="white")
