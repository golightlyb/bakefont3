import freetype
import math # ceil
import bakefont3.encode as bfencode
import bakefont3.decode as bfdecode


class font:
    """Bakefont3's representation of a freetype font"""

    @property
    def name(self):
        return self._name

    @property
    def face(self):
        return self._face

    def encode(self, index, size):
        """encodes to a byte structure"""

        face = self.face
        fsize = bfdecode.unfp26_6(size) # size as float
        assert face.is_scalable

        def relative(value):
            # value is in relative Font Units, so converted into pixels for the
            # given font rasterisation size
            return (float(value) * fsize) / float(face.units_per_EM)


        # https://www.freetype.org/freetype2/docs/tutorial/step1.html
        # https://www.freetype.org/freetype2/docs/tutorial/step2.html
        # https://www.microsoft.com/typography/otspec/TTCH01.htm

        # the generic name given by the user of bakefont3, e.g. "Title Font"
        yield bfencode.b8string(self.name)

        # the actual font family the font was based on e.g. "Liberation Mono"
        # NOTE - encoding unknown, don't trust the value
        yield bfencode.uint8(len(face.family_name))
        yield face.family_name
        yield b'\0'

        # (Font, Size) pair Index
        yield bfencode.uint32(index) # 0, 1, 2, ... n-1

        # flags (8 bytes) - ignore any null bytes, accept any order
        yield b'M' if face.is_fixed_width else b'm' # monospace
        yield b'K' if face.has_kerning    else b'k'
        yield b'H' if face.has_horizontal else b'h' # e.g. most Latin languages
        yield b'V' if face.has_vertical   else b'v' # e.g. some East Asian
        yield b'A' # antialiasing
        yield b'\0\0\0' # 3 placeholders

        # Font Units per EM.
        # normally always 2048 for TrueType, but can be a weird value such as 1
        # e.g. a square of 2048x2048, if the EM square is always the exact
        # same height as the font size. Hence, the size of each square
        # stretches with size.
        # You won't need this for a rasterised font!
        # yield bfencode.uint16(face.units_per_EM)

        # face.ascender
        # face.descender
        # https://www.microsoft.com/typography/otspec/os2.htm#sta
        # Don't use these - they are unreliable and you want the values per
        # glyph instead.

        # face.height - fixed point 26.6 (divide by 64 to get a float)
        # is an appropriate default line spacing
        # (no guarantee that no glyphs extend above or below baselines)
        yield bfencode.fp26_6(relative(face.height)) # in pixels

        # face.bbox
        # this is the size of a bounding box able to hold any glyph
        # NOTE - this is just a hint! After rendering, individual glyphs may
        # not match this!
        # face.bbox.xMin/yMin/xMax/yMax - in relative Font Units

        # TODO calculate this ourselves from our rasterised glyphs!
        yield bfencode.int16(0) # xMin placeholder
        yield bfencode.int16(0) # yMin placeholder
        yield bfencode.int16(0) # xMax placeholder
        yield bfencode.int16(0) # yMax placeholder

        # max_advance_width
        # maximum horizontal cursor advance
        # can be used to quickly compute the maximum advance width of a string
        # of text. Doesn't correspond to the maximum glyph image width!

        # TODO calculate this ourselves from our rasterised glyphs!
        # TODO height as well (for vertical fonts)
        yield bfencode.int16(0) # advance width placeholder
        yield bfencode.int16(0) # advance height placeholder

        # face.underline_position
        # vertical position, relative to the baseline, of the underline bar's
        # center. Negative if it is below the baseline.
        yield bfencode.fp26_6(relative(face.underline_position))

        # face.underline_thickness
        # vertical thickness of the underline (remember its centered)
        yield bfencode.fp26_6(relative(face.underline_thickness))


    @property
    def dpi(self):
        # typographic dpi is always 72. At 72ppi, 1pt == 1px
        self._dpi = 72

    def setSize(self, px_fp):
        # N.B. this method changes the internal freetype font state
        assert type(px_fp) is int
        # px_fp is 26.6 floating point encoding
        self.face.set_char_size(px_fp, 0, self.dpi, 0)

    def __init__(self, name, path):
        self._name = name
        self._path = path
        self._face = freetype.Face(path)
        self._size = 0
