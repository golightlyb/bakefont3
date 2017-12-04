import bakefont3 as bf
import numpy as np
from PIL import Image
import copy


def size_seq():
    # an infinite sequence of doubling square size pairs
    size = 64
    while True:
        yield (size, size)
        size *= 2

        # don't make anything ludicrously large
        if size > (32*1024): raise StopIteration;


class pack:
    @property
    def size(self):
        return self._size

    def image(self):
        width, height = self.size

        # create a greyscale image for each channel i.e. z-layer
        channels = [
            Image.new("L", (width, height), 0),
            Image.new("L", (width, height), 0),
            Image.new("L", (width, height), 0),
            Image.new("L", (width, height), 0),
        ]

        for g in self._glyphs:
            if not g.image: continue
            channels[g.z].paste(g.image, (g.x, g.y, g.x + g.width, g.y + g.height))

        # convert each channel to a numpy array
        img8 = [None] * 4
        for i in range(0, 4):
            data = channels[i].getdata()
            img = np.fromiter(data, np.uint8)
            img = np.reshape(img, (channels[i].height, channels[i].width))
            img8[i] = img

        # merge each channel into a RGBA image
        image = np.stack((img8[0], img8[1], img8[2], img8[3]), axis=-1)

        image = Image.fromarray(image, mode='RGBA')
        return image


    def data(self):
        # sort by id for indexing
        self._glyphs.sort(key=lambda glyph: glyph.id64)




    def __init__(self, renderResults, sizes=None):
        if sizes is None: sizes = size_seq()
        self._glyphs = []
        self._fonts = [] # font names
        self._fontpairs = set()
        if not renderResults: return

        # ordered list of font names
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
            self._fontpairs.add((size << 32) + (fontId << 48))

            for glyph in render.glyphs:
                #key = "%d/%d/%d" % (fontId, size, glyph.code)
                key = glyph.code + (size << 32) + (fontId << 48)
                if not key in _seen:
                    # we stomp on the glyph datastructure so need a copy
                    # to avoid corrupting them if method calls are
                    # interleaved weirdly
                    newglyph = copy.copy(glyph)
                    if (glyph.image):
                        newglyph.image = glyph.image.copy()

                    glyph = newglyph
                    glyph.id64 = key # stash it
                    _glyphs.append(glyph)
                    _seen.add(key)

        self._fonts  = _fonts
        self._glyphs = _glyphs # we stomp on each glyph
        self._size   = 0

        # sort by height for packing
        _glyphs.sort(key=lambda glyph: glyph.height, reverse=True)

        for size in sizes:
            width, height = size

            if pack.couldMaybeFit(size, _glyphs):
                print("    fitting: trying size %dx%d" % (width, height))
                if pack.fit(size, _glyphs):
                    self._size = size
                    return
            else:
                print("    fitting: skip size %dx%d (would never fit)" % (width, height))
                continue

        raise RuntimeError("unable to find a fit!")

    @staticmethod
    def couldMaybeFit(size, glyphs):
        # quickly estimate if it will fit even in the best-case?
        totalarea = 0
        width, height = size
        for glyph in glyphs:
            area = glyph.width * glyph.height
            totalarea += area
        if (totalarea > width * height * 4):
            return False
        return True

    @staticmethod
    def fit(size, glyphs):
        if not glyphs: return True
        width, height = size

        spaces = bf.tritree(bf.bbox(0, 0, 0, width, height , 4)) # 4=RGBA channels

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
                # their image is invisible because if there's less information
                # in the alpha layer
                glyph.z = 3 - fit.z0

            # status
            count += 1
            pc = int(100.0 * (count / num))
            if (pc - last_pc >= 5):
                last_pc = pc
                print("    fitting: %d%%" % pc)

        return True

