import bakefont3 as bf

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
        pass

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
        spaces = bf.btree(bf.bbox(0, 0, 0, size, size , 4)) # 4=RGBA channels

        for glyph in glyphs:
            if glyph.width and glyph.height:
                fit = spaces.fit(glyph)
                if not fit: return False
                # stash the size
                glyph.x = fit.x0
                glyph.y = fit.y0
                glyph.z = fit.z0

        return True

