import bakefont3 as bf
from PIL import Image
import numpy as np


class glyph:
    __slots__ = ['code', 'x', 'y', 'z', 'id64', 'width', 'height', 'image',
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
        self.xoffset = glyph.bitmap_left
        self.yoffset = glyph.bitmap_top
        self.advance = bf.decode_fp26(glyph.advance.x)

        # placeholder values that get stomped on exclusively by bf.pack(...)
        self.x = 0
        self.y = 0
        self.z = 0 # layer i.e. RGBA channel 0,1,2,3
        self.id64 = 0


class render:
    @property
    def font(self):
        return self._font

    @property
    def glyphs(self):
        return self._glyphs

    @property
    def size(self):
        return self._size


    def __init__(self, font, size, charset):
        assert isinstance(font, bf.font)
        assert isinstance(charset, bf.charset)
        assert 1.0 < size < 256.0 # arbitrary sensible limit

        self._font = font
        self._size = size
        self._glyphs = []

        font.setSize(size)

        for i in charset.chars:
            self._glyphs.append(glyph(font, i))

