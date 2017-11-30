import bakefont3 as bf
import freetype
import math


class font:
    @property
    def name(self):
        return self._name

    @property
    def face(self):
        return self._face

    def ascender(self):
        return bf.decode_fp26(self.face.ascender) #+ve, round towards +infinity

    def descender(self):
        return bf.decode_fp26(self.face.descender) #-ve, round towards -infinity

    def ascenderPixels(self):
        return int(math.ceil(self.descender()))

    def descenderPixels(self):
        return int(math.floor(self.descender()))

    @property
    def dpi(self):
        # typographic dpi is always 72. At 72ppi, 1pt == 1px
        self._dpi = 72

    def setSize(self, px):
        self.face.set_char_size(bf.encode_fp26(px), 0, self.dpi, 0)

    def __init__(self, name, path):
        self._name = name
        self._path = path
        self._face = freetype.Face(path)
        self._size = 0