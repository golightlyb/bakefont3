import bakefont3 as bf
from PIL import Image
import numpy as np


class glyph:
    __slots__ = ['code', 'width', 'height', 'image',
                 'xoffset', 'yoffset', 'advance']

    @property
    def advancePixels(self):
        return int(round(self.advance))

    @property
    def char(self):
        return chr(self.code)

    def __init__(self, font, code):
        assert isinstance(font, bf.font)
        assert isinstance(code, int)

        self.code = code # unicode value

        font.face.load_char(self.char)
        glyph = font.face.glyph
        bitmap = glyph.bitmap
        width = bitmap.width
        height = bitmap.rows
        src = bitmap.buffer
        dest = None

        # encode as PIL image
        if (width > 0) and (height > 0):
            arr = np.zeros(shape=(height,width), dtype=np.uint8)
            for y in range(height):
                for x in range(width):
                    c = src[x + (y * bitmap.pitch)]
                    arr[y,x] = c

            dest = Image.fromarray(arr, mode="L")

        self.width = width
        self.height = height
        self.image = dest

        # kerning information, see:
        # https: // www.freetype.org / freetype2 / docs / tutorial / step2.html
        self.xoffset = glyph.bitmap_left
        self.yoffset = glyph.bitmap_top
        self.advance = glyph.advance.x # in fp26.6


class packedGlyph:
    __slots__ = ['glyph', 'x', 'y', 'z']

    def __getattr__(self, attr):
        return getattr(self.glyph, attr)

    def __init__(self, glyph):
        self.glyph = glyph
        self.x     = 0 # x pixel in texture atlas
        self.y     = 0 # y pixel in texture atlas
        self.z     = 0 # layer i.e. RGBA channel 0,1,2,3




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

