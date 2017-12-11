import bakefont3 as bf3
from PIL import Image
import numpy as np
import freetype


class Render(bf3.Cube):
    __slots__ = ['image', 'ftGlyph']

    def __init__(self, ftFace, codepoint, antialias=True):

        if antialias:
            ftFace.load_char(codepoint)
        else:
            flags = 0 \
                | freetype.FT_LOAD_RENDER \
                | freetype.FT_LOAD_TARGET_MONO \
                | freetype.FT_LOAD_FORCE_AUTOHINT
            # autohint makes monochrome look better
            ftFace.load_char(codepoint, flags=flags)

        glyph  = ftFace.glyph
        width  = glyph.bitmap.width
        height = glyph.bitmap.rows
        pitch  = glyph.bitmap.pitch
        src    = glyph.bitmap.buffer

        if (pitch < 0):
            raise NotImplemented("TODO handle negative pitch")

        if (width > 0) and (height > 0):
            arr = np.zeros(shape=(height, width), dtype=np.uint8)

            if antialias:
                for y in range(height):
                    for x in range(width):
                        pixel = src[x + (y * pitch)]
                        arr[y, x] = pixel
            else:
                print(pitch)
                for y in range(height):
                    for x in range(width):
                        index = int(x/8) + (y * pitch)
                        pixel = src[index]
                        mask = 0x1 << (7 - (x % 8))
                        print("%d, %d, %d, %d, %d" % (x, index, pixel, mask, pixel & mask))
                        pixel &= mask
                        if pixel: arr[y, x] = 255

            super().__init__(0, 0, 0, width, height, 1)
            self.image = Image.fromarray(arr, mode="L")
        else:
            super().__init__(0, 0, 0, 0, 0, 0)
            self.image = None

        self.ftGlyph = ftFace.glyph


class Glyph(bf3.Cube):
    __slots__ = ['codepoint', 'render']

    def __getattr__(self, attr):
        return getattr(self.render, attr)

    @property
    def char(self):
        return chr(self.codepoint)

    def __init__(self, codepoint, render):
        assert isinstance(codepoint, int) # unicode value
        assert isinstance(render, Render)

        super().__init__(0, 0, 0, 0, 0, 0)
        self.codepoint = codepoint
        self.render = render
