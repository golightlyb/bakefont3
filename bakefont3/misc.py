
def encode_fp26(native_num):
    """
    Encode a number in 26.6 fixed point arithmatic with the lower 6 bytes
    used for the fractional component and the upper 26 bytes used for the
    integer component, returning a native int.

    Freetype uses this encoding to represent floats.
    """
    if isinstance(native_num, int):
        return (native_num << 6)
    else:
        return int(float(native_num) * 64.0)


def decode_fp26(num):
    """
    Decode a number from 26.6 fixed point arithmatic with the lower 6 bytes
    used for the fractional component and the upper 26 bytes used for the
    integer component, returning a native float.

    Freetype uses this encoding to represent floats.
    """
    return (float(num) / 64.0)


