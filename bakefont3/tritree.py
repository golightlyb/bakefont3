class bbox:
    """A bounding box / bounding cube!"""
    __slots__ = ['x0', 'y0', 'x1', 'y1', 'z0', 'z1']

    # bounding box
    def __init__(self, x0, y0, z0, x1, y1, z1):
        self.x0 = x0
        self.y0 = y0
        self.x1 = x1
        self.y1 = y1
        self.z0 = z0
        self.z1 = z1

    def set(self, x0, y0, x1, y1, z0, z1):
        self.x0 = x0
        self.y0 = y0
        self.x1 = x1
        self.y1 = y1
        self.z0 = z0
        self.z1 = z1

    @property
    def width(self):
        return (self.x1 - self.x0)

    @property
    def height(self):
        return (self.y1 - self.y0)

    @property
    def depth(self):
        return (self.z1 - self.z0)


class tritree:
    """
    Trinary tree node where each node also has a bounding box interface.

    If the node has no children, its bounding box represents empty space.

    Otherwise, its bounding box is split below and to the right and
    outwards by exactly three children, for which the same definition applies
    recursively.
    """

    __slots__ = ['bbox', 'right', 'down', 'out']

    @property
    def x0(self):
        return self.bbox.x0

    @property
    def x1(self):
        return self.bbox.x1

    @property
    def y0(self):
        return self.bbox.y0

    @property
    def y1(self):
        return self.bbox.y1

    @property
    def z0(self):
        return self.bbox.z0

    @property
    def z1(self):
        return self.bbox.z1

    @property
    def width(self):
        return self.bbox.width

    @property
    def height(self):
        return self.bbox.height

    @property
    def depth(self):
        return self.bbox.depth

    def __init__(self, starting_bbox):
        assert isinstance(starting_bbox, bbox)
        self.bbox  = starting_bbox
        self.right = None
        self.down  = None
        self.out   = None

    def isEmpty(self):
        return (self.right is None) and (self.down is None) and (self.out is None)

    def freeLayer(self):
        return self.z < 4

    def fit(self, item):
        """
        If empty: fit an item into this node, splitting it into three empty
        child nodes.

        If full: recursively try to fit it into its child nodes.

        The `item` argument is anything with a width and a height. Depth is
        assumed to be always 1.
        """
        # Returns False or the bbox that the item can fit into
        # item is anything with a width and height property

        if not self.isEmpty():
            # recurse down to a leaf with empty space and attempt to fit
            fit_right = self.right.fit(item)
            if fit_right: return fit_right

            fit_down = self.down.fit(item)
            if fit_down: return fit_down

            fit_out = self.out.fit(item)
            if fit_out: return fit_out

            # no room in any leaf
            return False

        # this node is empty, so attempt to fit the given bounding box
        w = item.width
        h = item.height

        # Doesn't fit
        if (w > self.width):    return False
        if (h > self.height):   return False
        if self.depth < 1:      return False

        # it fits, so split the remaining space into two bounding boxes
        # given that the list of inputs to fit is sorted on descending height,
        # we can maximise height by splitting on the bottom edge first

        right  = bbox(self.x0 + w, self.y0,     self.z0, self.x1,     self.y0 + h, self.z0 + 1)
        down   = bbox(self.x0,     self.y0 + h, self.z0, self.x1,     self.y1,     self.z0 + 1)
        out    = bbox(self.x0,     self.x0,     self.z0 + 1, self.x1,     self.y1, self.z1)
        fitbox = bbox(self.x0,     self.y0,     self.z0, self.x0 + w, self.y0 + h, self.z0 + 1)

        self.right = tritree(right)
        self.down  = tritree(down)
        self.out   = tritree(out)

        return fitbox
