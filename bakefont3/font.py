import freetype
import math # ceil, floor


class font:
    """Bakefont3's representation of a freetype font"""

    @property
    def name(self):
        return self._name

    @property
    def face(self):
        return self._face

    def bytes(self):
        """encodes to a byte structure"""
        # https://www.freetype.org/freetype2/docs/tutorial/step2.html
        # face.ascender # fixed point 26.6 encoding
        #face.descender # fixed point 26.6 encoding
        face = self.face
        print ('units per em:        {}'.format(face.units_per_EM))
        print ('ascender:            {}'.format(face.ascender))
        print ('descender:           {}'.format(face.descender))
        print ('height:              {}'.format(face.height))
        print ('bbox:                {}'.format([face.bbox.xMin, face.bbox.yMin, face.bbox.xMax, face.bbox.yMax]))
        print ('')
        print ('max_advance_width:   {}'.format(face.max_advance_width))
        print ('max_advance_height:  {}'.format(face.max_advance_height))
        print ('')
        print ('underline_position:  {}'.format(face.underline_position))
        print ('underline_thickness: {}'.format(face.underline_thickness))
        print ('')
        print ('Has horizontal:      {}'.format(face.has_horizontal))
        print ('Has vertical:        {}'.format(face.has_vertical))
        print ('Has kerning:         {}'.format(face.has_kerning))
        print ('Is fixed width:      {}'.format(face.is_fixed_width))
        print ('Is scalable:         {}'.format(face.is_scalable))



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
