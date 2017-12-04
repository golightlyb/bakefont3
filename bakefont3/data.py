import struct
from functools import reduce


"""
File Format
===========

All numbers are little-endian encoded.

Header
------

The file begins with the 12 byte header, consisting of the 11 ascii characters
"BAKEFONT3r0" and a null terminator for the 12th byte.

Immediately following this is the `uint16` pixel width of the texture atlas
and the `uint16` pixel height of the texture atlas.

Font table
---------

Following this is a 4 byte marker, the 4 ascii bytes `\nFNT`

Immediately following is a `uint16` containing the number of font names,
then a `uint16` of the byte size of the font table structure that immediately
follows.

The font table is a tightly packed list of records without padding.

Each record is, in order:

* the uint16 size of the record
* the uint16 size in bytes of the utf-8 encoded font name
* the utf-8 encoded name given to the font (e.g. "sans")
    (this will probably differ from its brand name)
* a null-terminator byte '\0'
* a sorted ascending list of uint16-encoded sizes of the font (note that the
  length of this can be calculated in advance from the previous information)

The uint16-encoded sizes are a precise integer encoding of a size that may
be fractional e.g. `11.5`. To get the actual value, simply divide by 64.

The id of a font is its position in the font table (0-indexed). For example,
the first font has Font ID 0. The second font has Font ID 1. The nth font
has Font ID n-1.


Glyph table
-----------

Following this is a 4 byte marker, the 4 ascii bytes `\nGLY`

The table is a sorted list of, in order, the following 26-byte structure:

* 1x uint64, the glpyh ID
* 2x int16: the xoffset, yoffset (kerning information)
* 1x uint32: the uint32-encoded advance (kerning information) - divide this
    by 64 to get a float.
* 3x uint16: the x,y,z offset into the texture atlas,
    where z is the channel (0: red, 1: green, 2: blue, 3: alpha)
    and x, y are pixels
* 2x uint16: the glpyh width and height in pixels

If the glyph width or height are zero, the glyph has no renderable image
and the x,y,z offsets will all be zero.

The glyph ID is:

* the Unicode value of the character
* plus the uint16-encoded size (from float: `int(size * 64.0`) bitshifted left 32 bits
* plus the font ID bitshifted left 48 bits

Thus, given a Unicode character, a font ID from the font table, and a size
that the font supports, you can perform a binary search on this sorted
list to find information about an appropriate glyph to display.

One further optimisation an implementation may make is to create a mapping
from font + size pairs to the range of glyph IDs. This may be useful for
specialised fonts that only use a few characters e.g. a font for a FPS counter.

"""



def flatten(it):
    seq = [x for x in it]
    return bytes(reduce(lambda x, y: x + y, seq))


class encode:
    """
    Encodes font and character and glyph and kerning information into a
    binary file. You will still need the separate texture atlas as well.
    """

    def __init__(self, size, fonts, sizes, glyphs):
        def _magic():
            yield b'BAKEFONT3r0\0'; # 12 bytes

            # 2x2 bytes = 4 bytes
            width, height = size
            yield struct.pack('<HH', width, height);
            yield b'\nFNT'

        def _fonts():
            # an indexed array of fonts
            numFonts = len(fonts)
            bufSize = 0
            for font in fonts:
                # 2 byte: length of row
                # 2 byte: length of font name
                bufSize += 2 + 2 + len(font) + 1 + 2
                # name of font
                # 1 byte: null terminated
                # 2 bytes * number of sizes: sizes the font supports

            yield struct.pack('<HH', numFonts, bufSize); # uint16, uint16 (4 bytes total)

            index = 0
            for font in fonts:
                name = font.encode('utf-8')
                rowsize = 2 + len(name) + 1 + 2 + len(sizes[index])
                assert b'\0' not in name
                yield struct.pack('<HH', rowsize, len(name));
                yield name + b'\0';
                for size in sorted(sizes[index], key=float):
                    yield struct.pack('<H', int(size * 64.0));
                index += 1

        parts = [_magic(), _fonts()]

        # concatenate
        self.data = flatten([flatten(x) for x in parts])


    def save(self, filename):
        with open(filename, 'wb') as fp:
            fp.write(self.data)

