#!/usr/bin/env python3
"""Audit a user-supplied Minecraft 1.2.5 JAR without modifying or bundling it."""
from __future__ import annotations

import argparse
import collections
import struct
import sys
import zipfile


class ClassFormatError(Exception):
    pass


class Reader:
    def __init__(self, data: bytes):
        self.data = data
        self.pos = 0

    def take(self, size: int) -> bytes:
        if size < 0 or self.pos + size > len(self.data):
            raise ClassFormatError("truncated class file")
        value = self.data[self.pos:self.pos + size]
        self.pos += size
        return value

    def u1(self) -> int:
        return self.take(1)[0]

    def u2(self) -> int:
        return struct.unpack(">H", self.take(2))[0]

    def u4(self) -> int:
        return struct.unpack(">I", self.take(4))[0]


def inspect_class(data: bytes) -> tuple[int, bool]:
    r = Reader(data)
    if r.take(4) != b"\\xca\\xfe\\xba\\xbe":
        raise ClassFormatError("bad class-file magic")
    r.u2()  # minor
    major = r.u2()
    count = r.u2()
    cp: list[object | None] = [None] * count
    i = 1
    while i < count:
        tag = r.u1()
        if tag == 1:
            cp[i] = r.take(r.u2()).decode("utf-8", "replace")
        elif tag in (3, 4):
            r.take(4)
        elif tag in (5, 6):
            r.take(8)
            i += 1
        elif tag in (7, 8, 16, 19, 20):
            cp[i] = (tag, r.u2())
        elif tag in (9, 10, 11, 12, 17, 18):
            cp[i] = (tag, r.u2(), r.u2())
        elif tag == 15:
            r.take(3)
        else:
            raise ClassFormatError(f"unknown constant-pool tag {tag}")
        i += 1

    r.take(6)  # access_flags, this_class, super_class
    for _ in range(r.u2()):
        r.take(2)  # interfaces

    def skip_members(count_members: int, check_main: bool = False) -> bool:
        found = False
        for _ in range(count_members):
            access, name_index, desc_index, attr_count = (
                r.u2(), r.u2(), r.u2(), r.u2()
            )
            name = cp[name_index] if name_index < len(cp) else None
            desc = cp[desc_index] if desc_index < len(cp) else None
            if check_main and name == "main" and desc == "([Ljava/lang/String;)V":
                found = bool(access & 0x0001 and access & 0x0008)
            for _ in range(attr_count):
                r.u2()
                r.take(r.u4())
        return found

    skip_members(r.u2())  # fields
    has_main = skip_members(r.u2(), check_main=True)
    return major, has_main


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("jar", help="path to the user's minecraft.jar")
    args = parser.parse_args()

    try:
        with zipfile.ZipFile(args.jar) as jar:
            class_entries = [n for n in jar.namelist() if n.endswith(".class")]
            versions: collections.Counter[int] = collections.Counter()
            main_present = False
            for entry in class_entries:
                major, has_main = inspect_class(jar.read(entry))
                versions[major] += 1
                if entry == "net/minecraft/client/Minecraft.class":
                    main_present = has_main

            print(f"Class files: {len(class_entries)}")
            print("Class-file major versions: " + ", ".join(
                f"{version}={count}" for version, count in sorted(versions.items())
            ))
            print("Minecraft entry point: " + (
                "public static main(String[]) found" if main_present else "NOT FOUND"
            ))
            for dependency in (
                "org/lwjgl/opengl/Display.class",
                "org/lwjgl/Sys.class",
            ):
                print(f"{dependency}: " + (
                    "present" if dependency in jar.namelist() else "not in this JAR"
                ))
            if not main_present:
                return 2
            return 0
    except (OSError, zipfile.BadZipFile, ClassFormatError, KeyError) as exc:
        print(f"JAR audit failed: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
