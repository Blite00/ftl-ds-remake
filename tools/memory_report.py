#!/usr/bin/env python3
"""Read ARM9 ELF static allocations; never executes the ROM."""
import argparse
import os
from pathlib import Path
import struct
import subprocess
import tempfile
ROOT = Path(__file__).resolve().parents[1]
BIN = ROOT / '.tools/msys64/opt/wonderful/toolchain/gcc-arm-none-eabi/bin'
os.environ['PATH'] = str(ROOT/'.tools/msys64/ucrt64/bin') + os.pathsep + str(ROOT/'.tools/msys64/usr/bin') + os.pathsep + str(BIN) + os.pathsep + os.environ.get('PATH','')

def sections(path):
    data = path.read_bytes()
    if data[:6] != b'\x7fELF\x01\x01':
        raise ValueError('Expected little-endian ELF32')
    shoff = struct.unpack_from('<I', data, 32)[0]
    entsize, count, names_index = struct.unpack_from('<HHH', data, 46)
    rows = [struct.unpack_from('<10I', data, shoff+i*entsize) for i in range(count)]
    nr = rows[names_index]
    names = data[nr[4]:nr[4]+nr[5]]
    return [(names[r[0]:].split(b'\0')[0].decode(), r[3], r[5], r[2]) for r in rows]

def state_sizes():
    # ARM ABI sizeof marker; compile only, no test execution.
    with tempfile.TemporaryDirectory(prefix='ftl-memory-', dir=ROOT/'build') as directory:
        src, obj = Path(directory)/'size.c', Path(directory)/'size.o'
        src.write_text('#include "campaign.h"\nchar game_size_marker[sizeof(Game)];\nchar campaign_size_marker[sizeof(Campaign)];\n')
        subprocess.run([str(BIN/'arm-none-eabi-gcc.exe'), '-c', str(src), '-I', str(ROOT/'source'), '-o', str(obj)], check=True, capture_output=True, timeout=30)
        # Read ELF32 symbol sizes directly. No separate nm process or DLL dependency.
        data=obj.read_bytes()
        if data[:6]!=b'\x7fELF\x01\x01':raise ValueError('Expected little-endian ARM ELF32 object')
        shoff=struct.unpack_from('<I',data,32)[0]
        entsize,count=struct.unpack_from('<HH',data,46)
        rows=[struct.unpack_from('<10I',data,shoff+i*entsize) for i in range(count)]
        sizes={}
        for row in rows:
            if row[1]!=2:continue
            strings=rows[row[6]];names=data[strings[4]:strings[4]+strings[5]]
            for pos in range(row[4],row[4]+row[5],row[9]):
                name,_,size,_,_,_=struct.unpack_from('<IIIBBH',data,pos)
                label=names[name:].split(b'\0')[0].decode()
                if label.endswith('_size_marker'):sizes[label.removesuffix('_size_marker')]=size
        if set(sizes)!={'game','campaign'}:raise ValueError('Missing ARM ABI markers')
        return sizes


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('elf', nargs='?', type=Path, default=ROOT/'build/ftl-ds.elf')
    args = parser.parse_args()
    print('STATIC ARM9 REPORT (not runtime free memory or a hardware benchmark)')
    print('ELF:', args.elf)
    ranges = []
    for name, address, size, flags in sections(args.elf):
        if flags & 2 and size:
            print('%-18s %8d bytes @ 0x%08x' % (name, size, address))
            if 0x02000000 <= address < 0x02400000:
                ranges.append((address,address+size))
    union, end = 0, 0
    for low, high in sorted(ranges):
        union += max(0, high-max(low,end))
        end = max(end,high)
    print('Main-RAM allocated section union:', union, 'bytes')
    print('Main-RAM image high-water offset:', end-0x02000000, 'bytes')
    print('DS main-RAM physical ceiling: 4194304 bytes; NOT all available to ARM9 heap.')
    print('Two CPU framebuffers: 196608 bytes (already counted in ELF .bss).')
    print('Two VRAM bitmap banks: 262144 bytes (separate from main RAM).')
    sizes = state_sizes()
    print('ARM ABI state sizes:', sizes, 'bytes (static Campaign including Game is already in .bss).')
    print('Temporary Game/Campaign copies on stack are additional; inspect nested call paths.')
    print('Excluded: ARM7 reservations, dynamic allocations, stack high-water, SD/filesystem buffers.')

if __name__ == '__main__':
    main()
