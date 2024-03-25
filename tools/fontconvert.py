import argparse
import freetype
from dataclasses import dataclass
from pathlib import Path

@dataclass
class FontDesc:
    name = ""
    first: int = 0
    last: int = 0
    baseline: int = 0
    y_advance: int = 0

@dataclass
class GlyphDesc:
    bitmap_offset: int = 0
    width: int = 0
    height: int = 0
    x_advance: int = 0
    x_offset: int = 0
    y_offset: int = 0

class BitmapEncoder:
    buffer = None
    acc = 0
    pos = 0

    def __init__(self, buf):
        self.buffer = buf
    
    def append(self, bit):
        if bit != 0:
            self.acc |= (0x80 >> self.pos)
        
        self.pos = self.pos + 1
        
        if self.pos >= 8:
            self.buffer.append(self.acc)
            self.acc = 0
            self.pos = 0

    def finalize(self):
        if self.pos != 0:
            self.buffer.append(self.acc)

def glyph_write(glyph, data):
    glyph_desc = GlyphDesc()
    glyph_desc.bitmap_offset = len(data)
    glyph_desc.width = glyph.bitmap.width
    glyph_desc.height = glyph.bitmap.rows
    glyph_desc.x_advance = int(glyph.advance.x / 64)
    glyph_desc.x_offset = glyph.bitmap_left
    glyph_desc.y_offset = 1 - glyph.bitmap_top

    encoder = BitmapEncoder(data)

    for i in range(glyph_desc.height):
        for j in range(glyph_desc.width):
            off = int(j / 8)
            mask = 0x80 >> (j & 7)
            encoder.append((glyph.bitmap.buffer[(i * glyph.bitmap.pitch) + off] & mask))

    encoder.finalize()
    return glyph_desc

def write_font(font_desc, glyph_descs, data):
    with open(f"{font_desc.name}.c", 'w') as f:
        f.write('#include "font.h"\n\n')
        f.write(f'const uint8_t {font_desc.name}_bitmaps[] = {{')
        for i, b in enumerate(data):
            if ((i % 12) == 0):
                f.write("\n  ")  
            f.write("0x{:02x}, ".format(b))
        f.write("\n};\n\n")

        f.write(f'const glyph_t {font_desc.name}_glyphs[] = {{')
        ch = font_desc.first
        for glyph in glyph_descs:
            f.write("\n  {{{: 5d}, {: 4d}, {: 4d}, {: 4d}, {: 4d}, {: 4d}}}, // 0x{:02x} '{:c}'".format(
                glyph.bitmap_offset, 
                glyph.width,
                glyph.height,
                glyph.x_advance,
                glyph.x_offset,
                glyph.y_offset,
                ch,
                ch
            ))
            ch = ch + 1
        f.write("\n};\n\n")
        f.write("const font_t {:s} = {{ {:s}_bitmaps, {:s}_glyphs, 0x{:02x}, 0x{:02x}, {:d}, {:d} }};\n".format(
            font_desc.name,
            font_desc.name,
            font_desc.name,
            font_desc.first,
            font_desc.last,
            font_desc.baseline,
            font_desc.y_advance
        ))

def main():
    parser = argparse.ArgumentParser(description='Convert font to bitmap')
    parser.add_argument('-f', '--font-file', help="the font file")
    parser.add_argument('-s', '--size', help="the output font size in pixels")
    parser.add_argument('-H', '--hinter', help="hinter (force-auto, no-auto)")
    args = parser.parse_args()
    
    font_desc = FontDesc()
    glyph_descs = []
    data = []

    font_face = freetype.Face(args.font_file)
    font_size = int(args.size)
    font_face.set_pixel_sizes(font_size, font_size)

    if args.hinter == "no-auto":
        hint_algo = freetype.FT_LOAD_NO_AUTOHINT        
    else:
        hint_algo = freetype.FT_LOAD_FORCE_AUTOHINT

    font_desc.y_advance = round(font_face.height / 64)
    font_desc.baseline = round(font_face.ascender / 64)
    ch = font_face.get_first_char()
    font_desc.first = ch[0]

    while ch[1] != 0:
        font_desc.last = ch[0]
        font_face.load_glyph(ch[1], freetype.FT_LOAD_RENDER | freetype.FT_LOAD_TARGET_MONO | hint_algo)
        glyph_descs.append(glyph_write(font_face.glyph, data))
        ch = font_face.get_next_char(ch[0], ch[1])

    font_desc.name = f"{Path(args.font_file).stem}_{font_size}px"
    write_font(font_desc, glyph_descs, data)

if __name__ == "__main__":
    main()