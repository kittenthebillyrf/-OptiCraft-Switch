#!/usr/bin/env python3
"""Audit an OpenJDK source ZIP for concrete Horizon/libnx port implementation.

The scanner intentionally excludes README/docs when looking for implementation
symbols, so prose claims cannot satisfy an implementation gate.
"""
from __future__ import annotations
import argparse
import json
import re
import zipfile
from pathlib import PurePosixPath

SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".h", ".hpp", ".inline.hpp", ".make"}
REQUIRED = {
    "horizon_code_memory": [r"svcManageCodeMemory", r"svcUnmapProcessCodeMemory"],
    "libnx_exception_bridge": [r"__libnx_exception_handler"],
    "switch_runner": [r"JNI_CreateJavaVM", r"JNI_GetDefaultJavaVMInitArgs"],
}
DOC_PARTS = {"readme", "docs", "doc", "todo", "license"}

def is_implementation_source(name: str) -> bool:
    p = PurePosixPath(name)
    low = [part.lower() for part in p.parts]
    if any(part in DOC_PARTS for part in low):
        return False
    if p.name.lower().startswith(("readme", "todo", "changelog")):
        return False
    return p.suffix.lower() in SOURCE_SUFFIXES

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("archive")
    ap.add_argument("--json", dest="json_path")
    args = ap.parse_args()
    found = {key: [] for key in REQUIRED}
    source_count = 0
    with zipfile.ZipFile(args.archive) as z:
        bad = z.testzip()
        if bad:
            raise SystemExit(f"corrupt ZIP member: {bad}")
        for info in z.infolist():
            if not is_implementation_source(info.filename):
                continue
            source_count += 1
            try:
                text = z.read(info).decode("utf-8", "ignore")
            except Exception:
                continue
            for key, patterns in REQUIRED.items():
                if all(re.search(pattern, text) for pattern in patterns):
                    found[key].append(info.filename)
    result = {
        "archive": args.archive,
        "source_files_scanned": source_count,
        "implementation_evidence": found,
        "gates": {key: bool(paths) for key, paths in found.items()},
        "ready_for_real_jvm_link": all(found.values()),
    }
    rendered = json.dumps(result, indent=2, ensure_ascii=False)
    print(rendered)
    if args.json_path:
        with open(args.json_path, "w", encoding="utf-8") as f:
            f.write(rendered + "\n")
    return 0 if result["ready_for_real_jvm_link"] else 2

if __name__ == "__main__":
    raise SystemExit(main())
