#!/usr/bin/env python3
"""Audit an OpenJDK source ZIP for concrete Horizon/libnx port implementation.

Documentation is excluded from implementation checks. The presence of generic
HotSpot JNI implementation does not count as a Switch-side application runner.
"""
from __future__ import annotations
import argparse
import json
import re
import zipfile
from pathlib import PurePosixPath

SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".h", ".hpp", ".make"}
DOC_PARTS = {"readme", "docs", "doc", "todo", "license"}

def is_implementation_source(name: str) -> bool:
    p = PurePosixPath(name)
    low = [part.lower() for part in p.parts]
    return (p.suffix.lower() in SOURCE_SUFFIXES
            and not any(part in DOC_PARTS for part in low)
            and not p.name.lower().startswith(("readme", "todo", "changelog")))

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("archive")
    ap.add_argument("--json", dest="json_path")
    args = ap.parse_args()
    evidence = {
        "horizon_code_memory": [],
        "libnx_exception_bridge": [],
        "jni_invocation_api": [],
        "switch_java_runner": [],
    }
    source_count = 0
    with zipfile.ZipFile(args.archive) as z:
        bad = z.testzip()
        if bad:
            raise SystemExit(f"corrupt ZIP member: {bad}")
        for info in z.infolist():
            if not is_implementation_source(info.filename):
                continue
            source_count += 1
            text = z.read(info).decode("utf-8", "ignore")
            if all(re.search(p, text) for p in
                   (r"svcManageCodeMemory", r"svcUnmapProcessCodeMemory")):
                evidence["horizon_code_memory"].append(info.filename)
            if "__libnx_exception_handler" in text:
                evidence["libnx_exception_bridge"].append(info.filename)
            if "JNI_CreateJavaVM" in text:
                evidence["jni_invocation_api"].append(info.filename)
                path_parts = [p.lower() for p in PurePosixPath(info.filename).parts]
                if any("runner" in p for p in path_parts) and re.search(
                    r"JNI_CreateJavaVM\s*\(", text
                ):
                    evidence["switch_java_runner"].append(info.filename)
    gates = {key: bool(paths) for key, paths in evidence.items()}
    result = {
        "archive": args.archive,
        "source_files_scanned": source_count,
        "implementation_evidence": evidence,
        "gates": gates,
        "ready_for_real_jvm_link": all(gates.values()),
    }
    rendered = json.dumps(result, indent=2, ensure_ascii=False)
    print(rendered)
    if args.json_path:
        with open(args.json_path, "w", encoding="utf-8") as f:
            f.write(rendered + "\n")
    return 0 if result["ready_for_real_jvm_link"] else 2

if __name__ == "__main__":
    raise SystemExit(main())
