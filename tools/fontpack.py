#!/usr/bin/env python3
import sys
from pathlib import Path
try:
    from PIL import Image, ImageDraw, ImageFont
except Exception:
    sys.exit(2)
if len(sys.argv)!=3:
    sys.exit(2)
font_path=Path(sys.argv[1]);out_path=Path(sys.argv[2])
font=ImageFont.truetype(str(font_path),14)
rows=[]
for code in range(32,127):
    im=Image.new('L',(12,16),0);d=ImageDraw.Draw(im)
    bbox=d.textbbox((0,0),chr(code),font=font)
    w=max(1,bbox[2]-bbox[0]);h=max(1,bbox[3]-bbox[1])
    x=max(0,(12-w)//2-bbox[0]);y=max(0,(16-h)//2-bbox[1])
    d.text((x,y),chr(code),font=font,fill=255)
    packed=[]
    for yy in range(16):
        bits=0
        for xx in range(12):
            if im.getpixel((xx,yy))>=96:
                bits|=1<<(11-xx)
        packed.append(bits)
    rows.append(packed)
out_path.parent.mkdir(parents=True,exist_ok=True)
with out_path.open('w',encoding='utf-8') as f:
    f.write('#pragma once\n#include <stdint.h>\nnamespace vazir_generated {\n')
    f.write('static const uint16_t glyphs[95][16] = {\n')
    for g in rows:
        f.write('  {'+','.join('0x%03X'%x for x in g)+'},\n')
    f.write('};\n}\n')
