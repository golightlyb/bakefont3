import bakefont3 as bf
import bakefont3.encode as bfencode
import numpy as np
from PIL import Image
import copy
import math


class saveable:
    def __init__(self, data):
        self.data = data
    def save(self, filename):
        with open(filename, 'wb') as fp:
            fp.write(self.data)



def size_seq():
    """an sequence of doubling square size pairs"""
    size = 64
    while True:
        yield (size, size)
        size *= 2

        # don't make anything ludicrously large
        if size > (32*1024): raise StopIteration;


class pack:
    """Once glyphs have been rasterised, this class sorts and fits them
    into an image.

        pack.data().save(file)
        pack.data().data => bytes
        pack.image().save(file)
        pack.image() => PIL Image (RGBA)
        pack.image().split() => a 4-tuple of greyscale PIL Images
                                for the channels Red, Green, Blue, Alpha
    """

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

        for g in self._allGlyphs:
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
        # returns a bytes object

        # sort by id so that its binary-searchable
        # self._glyphs.sort(key=lambda glyph: glyph.id64)

        texsize = self._size

        def header(fontsize, gsecsize):
            yield b'BAKEFONTv3r0'               # 12 bytes marker
            yield bfencode.uint16(texsize[0])   # texture atlas width
            yield bfencode.uint16(texsize[1])   # texture atlas height
            yield bfencode.uint32(48)           # absolute offset to font table
            yield bfencode.uint32(fontsize)     # size of font table in bytes
            yield bfencode.uint32(48+fontsize)  # absolute offset to glyph section
            yield bfencode.uint32(gsecsize)     # size of glyph section in bytes
            yield bfencode.uint32(48+fontsize+gsecsize) # absolute offset to kerning section
            yield bfencode.uint32(0)            # size of kerning section in bytes
            yield b'\0' * 12                    # 12 bytes reserved

        def font_table():
            yield b'FONTDATA' # 8 bytes marker
            yield bfencode.uint32(len(self._fonts))
            for fontId, (fontname, size) in enumerate(self._fontSizePairs):
                font = self._fonts[fontname]
                record = b''.join(font.encode(fontId, size))
                yield bfencode.uint32(len(record)) # size of font record in bytes
                yield record

        def glyph_section():

            yield b'GSETDATA'                       # 8 bytes marker

            # size in bytes of a glyph structure -   4 bytes
            yield bfencode.uint32(bf.packedGlyph.structSize)

            # number of glyph sets (always one for each font,size pair)
            assert len(self._fontSizePairs) == len(self._fonts)

            for fontId, (fontname, size) in enumerate(self._fontSizePairs):
                font = self._fonts[fontname]
                yield b'GSET'                       # 4 bytes marker
                yield bfencode.uint32(fontId)       # 4 bytes FontId
                glyphs = self._glyphs[fontId]
                yield bfencode.uint32(len(glyphs)) # number of glyphs

                # sort by ID for binary searcing
                glyphs.sort(key=lambda g: g.code)

                for glyph in glyphs:
                    yield from glyph.encode(font, size)


        font_table = b''.join(font_table())
        glyph_section = b''.join(glyph_section())

        header = b''.join(header(len(font_table), len(glyph_section)))

        return saveable(header + font_table + glyph_section)


    def __init__(self, renderResults, sizes=None):
        # sizes: sequence of 2-tuples of possible texture atlas (width, height)
        #        that should be tried. Defaults to a sequence [(2^n, 2^n), ...]
        assert renderResults is not None
        if sizes is None: sizes = size_seq()

        # build an ordered list of (font-name, size) tuples
        # and a dict of font-name -> font
        fontSizePairs = set()
        fonts = dict()
        for render in renderResults:
            fontSizePairs.add((render.font.name, render.size))
            fonts[render.font.name] = render.font

        fontSizePairs = sorted(list(fontSizePairs))

        # list of glyphs from every renderResult
        # -- ignore any duplicates of font+size+glyph
        uniqueGlyphs = {} # fontId => packedGlpyh
        allGlyphs = [] # sortable by height
        seenFontIds = set()
        for render in renderResults:
            fontId = fontSizePairs.index((render.font.name, render.size))

            for glyph in render.glyphs:
                key64 = glyph.code + (fontId << 32)

                if not key64 in seenFontIds:
                    if not fontId in uniqueGlyphs:
                        uniqueGlyphs[fontId] = []

                    packedGlyph = bf.packedGlyph(glyph)
                    uniqueGlyphs[fontId].append(packedGlyph)
                    allGlyphs.append(packedGlyph)
                    seenFontIds.add(key64)

        self._fonts = fonts
        self._fontSizePairs = fontSizePairs
        self._glyphs = uniqueGlyphs
        self._allGlyphs = allGlyphs
        self._size   = 0

        # sort by height for packing - good heuristic
        allGlyphs.sort(key=lambda glyph: glyph.height, reverse=True)

        for size in sizes:
            width, height = size

            if pack.couldMaybeFit(size, allGlyphs):
                print("    fitting: trying size %dx%d" % (width, height))
                if pack.fit(size, allGlyphs):
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
            if pc - last_pc >= 8:
                last_pc = pc
                print("    fitting: %d%%" % pc)

        return True

