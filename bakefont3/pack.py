import bakefont3 as bf
import numpy as np
from PIL import Image


def size_seq():
    # an infinite sequence of doubling sizes
    size = 64
    while True:
        yield size
        size *= 2

        # don't make anything ludicrously large
        if size > (32*1024): raise StopIteration;


class pack:
    @property
    def size(self):
        return self._size

    def image(self):
        channels = [
            Image.new("L", (self.size, self.size), 0),
            Image.new("L", (self.size, self.size), 0),
            Image.new("L", (self.size, self.size), 0),
            Image.new("L", (self.size, self.size), 0),
        ]

        for g in self._glyphs:
            if not g.image: continue
            channels[g.z].paste(g.image, (g.x, g.y, g.x + g.width, g.y + g.height))

        # convert each channel to a numpy array
        img8 = [None] * 4
        for i in range(0, 4):
            data = channels[i].getdata()
            img = np.fromiter(data, np.uint8)
            img = np.reshape(img, (channels[i].width, channels[i].height))
            img8[i] = img

        # merge each channel into a RGBA image
        image = np.stack((img8[0], img8[1], img8[2], img8[3]), axis=-1)

        image = Image.fromarray(image, mode='RGBA')
        return image


    def data(self):
        pass

    def __init__(self, renderResults):
        self._image = None
        self._glyphs = []
        if not renderResults: return

        # ordered list of fonts
        _fonts = []
        _seen = set()

        for render in renderResults:
            name = render.font.name
            if not name in _seen:
                _fonts.append(name)

        _fonts.sort(key=str)

        _glyphs = []
        _seen = set()

        # list of glyphs from every renderResult
        # -- ignore any duplicates of font+size+glyph
        for render in renderResults:
            fontId = _fonts.index(render.font.name)
            size   = render.size
            for glyph in render.glyphs:
                key = "%d/%d/%d" % (fontId, size, glyph.code)
                if not key in _seen:
                    _glyphs.append(glyph)
                    _seen.add(key)

        self._fonts  = _fonts
        self._glyphs = _glyphs
        self._size   = 0

        # sort by height for packing
        _glyphs.sort(key=lambda glyph: glyph.height, reverse=True)

        for size in size_seq():
            if pack.fit(size, _glyphs):
                self._size = size
                break


    @staticmethod
    def fit(size, glyphs):
        print("    fitting: trying size %dx%d" % (size, size))
        if not glyphs: return True

        spaces = bf.tritree(bf.bbox(0, 0, 0, size, size , 4)) # 4=RGBA channels

        num = len(glyphs)
        count = 0
        last_pc = -100

        for glyph in glyphs:
            if glyph.width and glyph.height:
                fit = spaces.fit(glyph)
                if not fit: return False
                # stash the size
                glyph.x = fit.x0
                glyph.y = fit.y0
                # we reverse the layers so that people don't think
                # their image is invisible because there's less information
                # in the alpha layer
                glyph.z = 3 - fit.z0

            # status
            count += 1
            pc = int(100.0 * (count / num))
            if (pc - last_pc >= 5):
                last_pc = pc
                print("    fitting: %d%%" % pc)

        return True

