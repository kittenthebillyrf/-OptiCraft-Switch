#!/usr/bin/env python3
"""Inspect the real Minecraft 1.2.5 main() entrypoint without executing it."""
import argparse
import struct
import zipfile

def u1(b, p): return b[p], p + 1
def u2(b, p): return struct.unpack_from(">H", b, p)[0], p + 2
def u4(b, p): return struct.unpack_from(">I", b, p)[0], p + 4

def parse_class(data):
    p = 0
    magic, p = u4(data, p)
    if magic != 0xCAFEBABE: raise ValueError("not a Java class file")
    minor, p = u2(data, p); major, p = u2(data, p)
    count, p = u2(data, p)
    cp = [None] * count
    i = 1
    while i < count:
        tag, p = u1(data, p)
        if tag == 1:
            n, p = u2(data, p); cp[i] = (tag, data[p:p+n].decode("utf-8", "replace")); p += n
        elif tag in (3, 4): cp[i] = (tag, data[p:p+4]); p += 4
        elif tag in (5, 6): cp[i] = (tag, data[p:p+8]); p += 8; i += 1
        elif tag in (7, 8, 16, 19, 20):
            x, p = u2(data, p); cp[i] = (tag, x)
        elif tag in (9, 10, 11, 12, 17, 18):
            a, p = u2(data, p); c, p = u2(data, p); cp[i] = (tag, a, c)
        elif tag == 15:
            a, p = u1(data, p); c, p = u2(data, p); cp[i] = (tag, a, c)
        else: raise ValueError("unknown constant-pool tag %d" % tag)
        i += 1
    access, p = u2(data,p); this_class,p=u2(data,p); super_class,p=u2(data,p)
    interfaces,p=u2(data,p); p += interfaces*2
    fields,p=u2(data,p)
    for _ in range(fields):
        p += 6; ac,p=u2(data,p)
        for __ in range(ac):
            ni,p=u2(data,p); ln,p=u4(data,p); p += ln
    methods,p=u2(data,p)
    found=None
    for _ in range(methods):
        flags,p=u2(data,p); name_i,p=u2(data,p); desc_i,p=u2(data,p); ac,p=u2(data,p)
        name=cp[name_i][1]; desc=cp[desc_i][1]
        for __ in range(ac):
            ni,p=u2(data,p); ln,p=u4(data,p); end=p+ln
            attr=cp[ni][1]
            if name=="main" and desc=="([Ljava/lang/String;)V" and attr=="Code":
                max_stack,p=u2(data,p); max_locals,p=u2(data,p); clen,p=u4(data,p)
                code=data[p:p+clen]; p += clen
                ex_count,p=u2(data,p); p += ex_count*8
                sub_count,p=u2(data,p)
                for ___ in range(sub_count):
                    _,p=u2(data,p); sl,p=u4(data,p); p += sl
                found=(flags,max_stack,max_locals,code)
            p=end
    if not found: raise ValueError("main([Ljava/lang/String;)V with Code attribute not found")
    return major, minor, cp, found

def cp_text(cp, idx):
    if idx <= 0 or idx >= len(cp) or cp[idx] is None: return "#%d" % idx
    item=cp[idx]; tag=item[0]
    if tag in (7,8,16,19,20):
        val=item[1]
        return cp[val][1] if cp[val] and cp[val][0]==1 else str(val)
    if tag in (9,10,11,12,17,18):
        a,b=item[1],item[2]
        if tag in (9,10,11):
            owner=cp_text(cp,a); nt=cp[b]
            return owner + "." + cp[nt[1]][1] + cp[nt[2]][1]
        return cp_text(cp,a) + ":" + cp_text(cp,b)
    return str(item[1:])

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("jar")
    args=ap.parse_args()
    with zipfile.ZipFile(args.jar) as jar:
        raw=jar.read("net/minecraft/client/Minecraft.class")
    major,minor,cp,method=parse_class(raw)
    flags,max_stack,max_locals,code=method
    print("class_version=%d.%d" % (major,minor))
    print("main_descriptor=([Ljava/lang/String;)V")
    print("main_code_bytes=%d max_stack=%d max_locals=%d" % (len(code),max_stack,max_locals))
    print("main_bytecode_hex="+code.hex())
    print("constant_pool_runtime_dependencies:")
    for i,item in enumerate(cp):
        if item and item[0] in (10,11):
            ref=cp_text(cp,i)
            if any(x in ref for x in ("java/lang/","org/lwjgl/")):
                print("  "+ref)
    print("NOTE: static analysis only; this script does not execute Java bytecode.")

if __name__=="__main__": main()
