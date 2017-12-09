import bakefont3 as bf
import bakefont3.encode as bfencode
import bakefont3.decode as bfdecode
from PIL import Image
import numpy as np


class render:
    @property
    def font(self):
        return self._font

    @property
    def glyphs(self):
        return self._glyphs

    @property
    def size(self):
        return self._size # fp26.6

    def __init__(self, font, size, charset):
        # size is a float e.g. "11.5"
        size = float(size)
        assert isinstance(font, bf.font)
        assert isinstance(charset, bf.charset)
        assert 1.0 < size < 255.0

        self._font = font
        self._glyphs = []
        self._size = int (size * 64.0) # fp26.6

        font.setSize(self._size)

        for i in charset.chars:
            self._glyphs.append(glyph(font, i))



class glyph:
    __slots__ = ['code', 'width', 'height', 'image', 'handle']

    @property
    def char(self):
        return chr(self.code)

    def __init__(self, font, code):
        assert isinstance(font, bf.font)
        assert isinstance(code, int)

        self.code = code # unicode value
        has_glyph = True

        if font.face.get_char_index(code):
            font.face.load_char(self.char)
            glyph = font.face.glyph
            bitmap = glyph.bitmap
            width = bitmap.width
            height = bitmap.rows
            src = bitmap.buffer
            dest = None

            # encode as PIL image
            if (width > 0) and (height > 0):
                arr = np.zeros(shape=(height, width), dtype=np.uint8)
                for y in range(height):
                    for x in range(width):
                        c = src[x + (y * bitmap.pitch)]
                        arr[y, x] = c

                dest = Image.fromarray(arr, mode="L")

            self.width = width
            self.height = height
            self.image = dest
            self.handle = font.face.glyph

        else:
            print("    notice: no glyph in font at Unicode code point 0x%x (%s)" % (code, repr(chr(code))))
            self.width = 0
            self.height = 0
            self.image = None
            self.handle = None


class packedGlyph:
    __slots__ = ['glyph', 'x', 'y', 'z']

    structSize = 36

    def __getattr__(self, attr):
        return getattr(self.glyph, attr)

    def __init__(self, glyph):
        self.glyph = glyph
        self.x     = 0 # x pixel in texture atlas
        self.y     = 0 # y pixel in texture atlas
        self.z     = 0 # layer i.e. RGBA channel 0,1,2,3

    def encode(self, font, size):
        def relative(value):
            # value is in relative Font Units, so converted into pixels for the
            # given font rasterisation size
            fsize = bfdecode.unfp26_6(size)  # size as float
            return (float(value) * fsize) / float(font.face.units_per_EM)

        # Unicode code point
        yield bfencode.uint32(self.code)    # 4 bytes

        # pixel position in texture atlas
        yield bfencode.uint16(self.x)       # 2 bytes
        yield bfencode.uint16(self.y)       # 2 bytes
        yield bfencode.uint8(self.z)        # 1 byte

        # pixel width in texture atlas
        yield bfencode.uint8(self.width)    # 1 byte
        yield bfencode.uint8(self.height)   # 1 byte
        yield b'\0'                         # 1 byte padding

        if self.handle:
            # horizontal left side bearing and top side bearing
            # positioning information relative to baseline
            yield bfencode.fp26_6(self.handle.metrics.horiBearingX) # 4 bytes
            yield bfencode.fp26_6(self.handle.metrics.horiBearingY) # 4 bytes
            # advance - how much to advance the pen by horizontally after drawing
            yield bfencode.fp26_6(self.handle.metrics.horiAdvance)  # 4 bytes

            yield bfencode.fp26_6(self.handle.metrics.vertBearingX)  # 4 bytes
            yield bfencode.fp26_6(self.handle.metrics.vertBearingY)  # 4 bytes
            yield bfencode.fp26_6(self.handle.metrics.vertAdvance)   # 4 bytes
        else:
            yield b'\0\0\0\0' * 6




