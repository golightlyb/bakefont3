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

def fixedstring(nativeString, bufsize, encoding="utf-8"):
    spare = bufsize - len(nativeString)
    assert spare >= 1
    bytes = nativeString.encode(encoding)
    return bytes + (b'\0' * spare)


def header(result):
    width, height, depth = result.size

    # Notation: `offset | size | notes`

    # HEADER - 16 byte block
    yield b"BAKEFONT30"   #  0 | 10 | magic bytes, version 3.0
    yield uint16(width)   # 10 |  2 | texture atlas width
    yield uint16(height)  # 12 |  2 | texture atlas height
    yield uint16(depth)   # 14 |  2 | texture atlas depth (1, 3, 4)


def fonts(result):
    # Notation: `offset | size | notes`

    # FONT TABLE HEADER - 8 bytes
    yield b"FONT"                   # 16 | 4 | debugging marker
    yield uint16(len(result.fonts)) # 18 | 2 | number of fonts
    yield b"\0\0"                   # 20 | 2 | padding (realign to 8 bytes)

    # FONT RECORDS - 48 bytes * number of fonts
    # (for n = 0; n => n + 1; each record is at offset 20 + 48n)
    # the FontID is implicit by the order e.g. the first font has FontID 0
    for font in result.fonts:
        name, _ = font
        yield fixedstring(name, 48) # 20+48n | 48 | name for font ID n


def modes(result):
    # Notation: `offset | size | notes`
    # offset is relative to 20 + (48 * number of fonts)

    # FONT MODE TABLE HEADER - 8 bytes
    yield b"MODE"                   # 6  | 4 | debugging marker
    yield uint16(len(result.modes)) # 18 | 2 | number of modes
    yield b"\0\0"                   # 20 | 2 | padding (realign to 8 bytes)

    # FONT MODE RECORDS
    # the ModeID is implicit by the order e.g. the first mode has ModeID 0
    for mode in result.modes:
        fontID, size, antialias = mode


def all(result):
    _header = b''.join(header(result))
    _fonts  = b''.join(fonts(result))
    _modes  = b''.join(modes(result))

    yield _header
    yield _fonts