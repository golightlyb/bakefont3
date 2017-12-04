
class charset:
    """Represents a set of characters, specified as ranges.

    The constructor takes a list of arguments:

    * a number - a Unicode code point value
    * a letter - a Unicode character value
    * a 2-tuple - a range between two values (inclusive)
    * a string - a string of Unicode characters
    * a bf.charset object - a previously defined set of values

    Because it is a set you don't have to worry about duplicate values.
    """

    @property
    def chars(self):
        return self._chars

    def __init__(self, *args, **kwargs):
        _chars = set()

        def norm(arg):
            if isinstance(arg, str) and len(arg) == 1:
                return ord(arg)
            elif isinstance(arg, int):
                return arg
            else:
                raise ValueError("Invalid range argument %s" % repr(arg))

        for arg in args:
            if isinstance(arg, str):
                _chars.update(map(ord, arg))
            elif isinstance(arg, int):
                _chars.update([arg])
            elif isinstance(arg, charset):
                _chars.update(arg.chars)
            elif isinstance(arg, tuple) and len(arg) == 2:
                left  = norm(arg[0])
                right = norm(arg[1])

                if (left > right):
                    left,right = right,left

                for i in range(left, right+1):
                    _chars.update([norm(i)])
            else:
                raise ValueError("Invalid argument %s" % repr(r))

        self._chars = _chars

