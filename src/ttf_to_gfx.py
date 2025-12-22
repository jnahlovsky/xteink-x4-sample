#!/usr/bin/env python3
"""
TrueType to Adafruit GFX font converter
Based on Adafruit's fontconvert tool

IMPORTANT: Adafruit GFX uses fully bit-packed bitmaps with NO per-scanline
padding. Bits flow continuously, only padding at the END of each glyph
to the next byte boundary.

# For bigger question text (currently 22pt → try 28pt or 32pt)
python src/ttf_to_gfx.py src/Lexend-Light.ttf 28 32 383 src/Lexend_Light28pt7b.h

# For bigger category text (currently 18pt → try 24pt)
python src/ttf_to_gfx.py src/Lexend-Bold.ttf 24 32 383 src/Lexend_Bold24pt7b.h
"""

import freetype
import sys

def convert_font(font_path, size, first_char, last_char, output_file=None):
    face = freetype.Face(font_path)
    face.set_pixel_sizes(0, size)

    # Handle both forward and backslashes for Windows/Unix paths
    font_name = font_path.replace('\\', '/').split('/')[-1].split('.')[0].replace('-', '_')

    # Open output file if provided
    if output_file:
        f = open(output_file, 'w', encoding='ascii')
    else:
        f = sys.stdout

    # Header includes
    f.write("#include <Adafruit_GFX.h>\n\n")

    # Bitmap array
    f.write(f"const uint8_t {font_name}{size}pt7bBitmaps[] PROGMEM = {{\n")

    bitmaps = []
    glyphs = []
    bitmap_offset = 0

    for char_code in range(first_char, last_char + 1):
        face.load_char(char_code, freetype.FT_LOAD_RENDER | freetype.FT_LOAD_TARGET_MONO)
        bitmap = face.glyph.bitmap

        # Adafruit GFX uses fully bit-packed bitmaps:
        # - Bits flow continuously with NO per-scanline padding
        # - Only pad at the END of each glyph to byte boundary

        # Collect all bits for this glyph
        bits = []
        for y in range(bitmap.rows):
            for x in range(bitmap.width):
                byte_index = y * bitmap.pitch + (x // 8)
                bit_index = 7 - (x % 8)  # MSB first
                if byte_index < len(bitmap.buffer):
                    pixel = (bitmap.buffer[byte_index] >> bit_index) & 1
                else:
                    pixel = 0
                bits.append(pixel)

        # Pack bits into bytes (8 bits per byte, MSB first)
        glyph_data = []
        for i in range(0, len(bits), 8):
            byte = 0
            for j in range(8):
                if i + j < len(bits):
                    byte |= bits[i + j] << (7 - j)
            glyph_data.append(byte)

        # Pad to byte boundary if needed (already handled by the packing above)
        # The (width * height + 7) // 8 formula ensures proper byte count

        bitmaps.extend(glyph_data)

        glyphs.append({
            'bitmap_offset': bitmap_offset,
            'width': bitmap.width,
            'height': bitmap.rows,
            'x_advance': face.glyph.advance.x >> 6,
            'x_offset': face.glyph.bitmap_left,
            'y_offset': 1 - face.glyph.bitmap_top  # Note: Adafruit uses 1 - top, not -top
        })

        bitmap_offset += len(glyph_data)

    # Print bitmap data
    for i, byte in enumerate(bitmaps):
        if i % 12 == 0:
            f.write("  ")
        f.write(f"0x{byte:02X}")
        if i < len(bitmaps) - 1:
            f.write(", ")
        if (i + 1) % 12 == 0:
            f.write("\n")
    f.write(" };\n\n")

    # Print glyph data
    f.write(f"const GFXglyph {font_name}{size}pt7bGlyphs[] PROGMEM = {{\n")
    for i, g in enumerate(glyphs):
        f.write(f"  {{ {g['bitmap_offset']:5}, {g['width']:3}, {g['height']:3}, "
              f"{g['x_advance']:3}, {g['x_offset']:4}, {g['y_offset']:4} }}")
        if i < len(glyphs) - 1:
            f.write(",")
        f.write(f"   // 0x{first_char + i:02X} '{chr(first_char + i) if 32 <= first_char + i <= 126 else '?'}'\n")
    f.write("};\n\n")

    # Print font structure
    # Use metrics.height if available (like original fontconvert), fallback to ascender-descender
    if face.size.height:
        yAdvance = face.size.height >> 6
    else:
        yAdvance = (face.size.ascender - face.size.descender) >> 6
    f.write(f"const GFXfont {font_name}{size}pt7b PROGMEM = {{\n")
    f.write(f"  (uint8_t  *){font_name}{size}pt7bBitmaps,\n")
    f.write(f"  (GFXglyph *){font_name}{size}pt7bGlyphs,\n")
    f.write(f"  0x{first_char:02X}, 0x{last_char:02X}, {yAdvance} }};\n\n")
    f.write(f"// Approx. {len(bitmaps) + len(glyphs) * 7 + 7} bytes\n")

    if output_file:
        f.close()

if __name__ == '__main__':
    if len(sys.argv) < 5 or len(sys.argv) > 6:
        print("Usage: python ttf_to_gfx.py font.ttf size first_char last_char [output.h]")
        print("Example: python ttf_to_gfx.py Lexend-Bold.ttf 18 32 255 Lexend_Bold18pt7b.h")
        sys.exit(1)

    font_path = sys.argv[1]
    size = int(sys.argv[2])
    first_char = int(sys.argv[3])
    last_char = int(sys.argv[4])
    output_file = sys.argv[5] if len(sys.argv) == 6 else None

    convert_font(font_path, size, first_char, last_char, output_file)
