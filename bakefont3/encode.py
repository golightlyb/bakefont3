import struct

ENDIAN = '<' # always little endian


def fp26_6(native_num):
    """
    Encode a number in 26.6 fixed point arithmatic with the lower 6 bytes
    used for the fractional component and the upper 26 bytes used for the
    integer component, returning a native int.

    Freetype uses this encoding to represent floats.
    """
    if isinstance(native_num, int):
        result = native_num * 64
    else:
        result = int(float(native_num) * 64.0)

    return int32(result)


def int8(num):
    assert num < 128
    return struct.pack(ENDIAN+'b', num);

def uint8(num):
    assert num < 256
    return struct.pack(ENDIAN+'B', num);

def int16(num):
    assert num < (128 * 256)
    return struct.pack(ENDIAN+'h', num);

def uint16(num):
    assert num < (256 * 256)
    return struct.pack(ENDIAN+'H', num);

def int32(num):
    return struct.pack(ENDIAN+'i', num);

def uint32(num):
    return struct.pack(ENDIAN+'I', num);

def int64(num):
    return struct.pack(ENDIAN+'q', num);

def uint64(num):
    return struct.pack(ENDIAN+'Q', num);

def cstring(nativeString, encoding="utf-8"):
    """null-terminated C-style string"""
    bytes = nativeString.encode(encoding);
    return bytes + b'\0';

def b8string(nativeString, encoding="utf-8"):
    """uint8 length-prefixed Pascal-style string
    plus a C-style null terminator
    aka a 'bastard string'."""
    bytes = nativeString.encode(encoding);
    length = len(bytes)
    assert length < 256
    return uint8(length) + bytes + b'\0';