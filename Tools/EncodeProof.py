#!/usr/bin/env python3
"""EncodeProof — one marked raw dump to one compressed PNG, stdlib only.

Usage: python3 Tools/EncodeProof.py <input.rgba> <output.png>
The dump carries the marker RIFTRAW1, the along extent and the across extent
before its RGBA ordinates.
"""

import struct
import sys
import zlib


def Encode(SourcePath, DestinationPath):
    with open(SourcePath, "rb") as Stream:
        Marker = Stream.read(8)
        if Marker != b"RIFTRAW1":
            raise SystemExit(f"{SourcePath}: not a marked raw dump")
        AlongExtent, AcrossExtent = struct.unpack("<II", Stream.read(8))
        Ordinates = Stream.read(AlongExtent * AcrossExtent * 4)
        if len(Ordinates) != AlongExtent * AcrossExtent * 4:
            raise SystemExit(f"{SourcePath}: ordinates short of the declared extent")

    # ① Filter 0 per scanline, zlib-compressed — the smallest honest PNG.
    Stride = AlongExtent * 4
    Scanlines = bytearray()
    for Across in range(AcrossExtent):
        Scanlines.append(0)
        Scanlines.extend(Ordinates[Across * Stride:(Across + 1) * Stride])
    Compressed = zlib.compress(bytes(Scanlines), 9)

    def Chunk(Tag, Body):
        CRC = zlib.crc32(Tag + Body) & 0xFFFFFFFF
        return struct.pack(">I", len(Body)) + Tag + Body + struct.pack(">I", CRC)

    Png = bytearray()
    Png += b"\x89PNG\r\n\x1a\n"
    Png += Chunk(b"IHDR", struct.pack(">IIBBBBB", AlongExtent, AcrossExtent, 8, 6, 0, 0, 0))
    Png += Chunk(b"IDAT", Compressed)
    Png += Chunk(b"IEND", b"")

    with open(DestinationPath, "wb") as Stream:
        Stream.write(Png)
    print(f"EncodeProof: {DestinationPath} ({AlongExtent}x{AcrossExtent}, {len(Png)} bytes)")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        raise SystemExit(__doc__)
    Encode(sys.argv[1], sys.argv[2])
