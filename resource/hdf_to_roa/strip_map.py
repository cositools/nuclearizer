import numpy as np


class StripMap:
    def __init__(self, fname, nAsicCh=32, nAsic=4, nStrip=64):
        self.nAsic = nAsic
        self.nAsicCh = nAsicCh
        self.nStrip = nStrip
        self.sides = []

        if fname != None:
            self.load_stripmap(fname)
        else:
            print("No filename loaded!")

    def get_side_strip(self, asic, ch):
        return self.stripmap[(asic, ch)]

    def get_asic_ch(self, side, strip):
        return self.chmap[(side, strip)]

    def load_stripmap(self, fname):
        temp_stripmap = {}
        temp_backend_stripmap = np.zeros((self.nAsic, self.nAsicCh), dtype=np.int32)
        temp_chmap = {}

        try:
            with open(fname, "r") as f:
                for line in f:
                    # Check for comments in the strip map file
                    if line.startswith("#"):
                        continue

                    # Split line by whitespace
                    tokens = line.split()
                    assert len(tokens) == 3
                    asic = int(tokens[0])
                    ch = int(tokens[1])
                    strip = tokens[2]
                    if strip != "AGND":
                        strip = int(strip)
                    side = int(asic / (self.nAsic / 2))
                    if not side in self.sides:
                        self.sides.append(side)
                    assert asic >= 0 and asic < self.nAsic
                    assert ch >= 0 and ch < self.nAsicCh
                    assert (strip == "AGND") or (strip >= 0 and strip < self.nStrip)
                    assert (asic, ch) not in temp_stripmap
                    assert (side, strip) not in temp_chmap

                    temp_stripmap[(asic, ch)] = (side, strip)
                    if strip == "AGND":
                        temp_backend_stripmap[asic][ch] = -1
                    else:
                        temp_backend_stripmap[asic][ch] = (side << 8) | strip
                    if strip != "AGND":
                        temp_chmap[(side, strip)] = (asic, ch)

        except Exception as e:
            print(
                'Strip Map File "{}" is not formatted correctly: {}'.format(
                    fname, str(e)
                )
            )
            return

        error = False
        for asic in range(self.nAsic):
            for ch in range(self.nAsicCh):
                if (asic, ch) not in temp_stripmap:
                    error = True
                    print(
                        'Strip Map File "{}" is not formatted correctly: Does not contain entry for ASIC {}, Ch{}'.format(
                            fname, asic, ch
                        )
                    )

        connected_strips = {}
        for side in self.sides:
            for strip in range(self.nStrip):
                connected_strips[(side, strip)] = (side, strip) in temp_chmap

        if not error:
            self.stripmap = temp_stripmap
            self.backendStripmap = temp_backend_stripmap
            self.chmap = temp_chmap
            self.connected_strips = connected_strips
            self.fname = fname

        return
