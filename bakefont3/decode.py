

ENDIAN = '<' # always little endian


def unfp26_6(num):
    return (float(num) / 64.0)

def fp26_6(num):
    """
    Decode a number from 26.6 fixed point arithmatic with the lower 6 bytes
    used for the fractional component and the upper 26 bytes used for the
    integer component, returning a native float.

    Freetype uses this encoding to represent floats.
    """

    assert type(num) is not bytes # TODO
    return (float(num) / 64.0)


