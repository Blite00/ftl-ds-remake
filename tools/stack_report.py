"""Static worst-case ARM9 stack estimate for the DS build.

Recompiles every source/*.c with the ROM's ARM9 flags plus -fstack-usage and
-fcallgraph-info=su into build/stack (the ROM build is not touched), then walks
the per-function call graph from main and the VBlank handler.  Frames are the
compiler's own stack-usage numbers.  Calls into libnds/maxmod/libc and indirect
calls are listed as unknown and add nothing, so the result is a lower bound on
those paths.  Recursion is reported rather than followed.

Usage: python tools/stack_report.py [output.txt]
"""
import glob, os, re, subprocess, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WF = os.path.join(ROOT, '.tools', 'msys64', 'opt', 'wonderful')
GCC = os.path.join(WF, 'toolchain', 'gcc-arm-none-eabi', 'bin', 'arm-none-eabi-gcc.exe')
SDK = os.path.join(WF, 'thirdparty', 'blocksds', 'core')
OUT = os.path.join(ROOT, 'build', 'stack')
FLAGS = ['-Wall', '-I' + os.path.join(ROOT, 'generated'), '-I' + os.path.join(SDK, 'libs', 'maxmod', 'include'),
         '-I' + os.path.join(SDK, 'libs', 'libnds', 'include'), '-D__NDS__', '-D__BLOCKSDS__', '-DARM9', '-mthumb',
         '-mcpu=arm946e-s+nofp', '-O2', '-ffunction-sections', '-fdata-sections',
         '-specs=' + os.path.join(SDK, 'sys', 'crts', 'ds_arm9.specs')]
ROOTS = ['main', 'capture_vblank', 'original_fill']  # main loop, VBlank input handler, Maxmod stream fill (runs inside mmStreamUpdate)
DTCM_STACK = 16 * 1024


def compile_all():
    os.makedirs(OUT, exist_ok=True)
    env = dict(os.environ)
    env['PATH'] = os.path.join(WF, 'toolchain', 'gcc-arm-none-eabi', 'bin') + os.pathsep + os.path.join(ROOT, '.tools', 'msys64', 'usr', 'bin') + os.pathsep + env['PATH']
    for src in sorted(glob.glob(os.path.join(ROOT, 'source', '*.c'))):
        base = os.path.join(OUT, os.path.basename(src))
        cmd = [GCC] + FLAGS + ['-fstack-usage', '-fcallgraph-info=su', '-c', '-o', base + '.o', src]
        subprocess.run(cmd, check=True, cwd=OUT, env=env)


def parse():
    frames, edges, dynamic = {}, {}, set()
    node = re.compile(r'node: \{ title: "([^"]+)" label: "([^"]*)"')
    edge = re.compile(r'edge: \{ sourcename: "([^"]+)" targetname: "([^"]+)"')
    for ci in glob.glob(os.path.join(OUT, '*.ci')):
        text = open(ci, encoding='utf-8', errors='replace').read()
        for title, label in node.findall(text):
            m = re.search(r'(\d+) bytes \((\w+)', label.replace('\\n', ' '))
            if m:
                frames[title] = max(frames.get(title, 0), int(m.group(1)))
                if m.group(2) != 'static':
                    dynamic.add(title)
        for a, b in edge.findall(text):
            edges.setdefault(a, set()).add(b)
    return frames, edges, dynamic


def worst(fn, frames, edges, stack, memo, notes):
    if fn in memo:
        return memo[fn]
    if fn in stack:
        notes['recursion'].add(fn)
        return (0, [])
    if fn not in frames:
        if fn != '__indirect_call':
            notes['external'].add(fn.split(':')[-1])
        else:
            notes['indirect'] += 1
        return (0, [])
    stack.add(fn)
    best = (0, [])
    for callee in edges.get(fn, ()):
        r = worst(callee, frames, edges, stack, memo, notes)
        if r[0] > best[0]:
            best = r
    stack.discard(fn)
    memo[fn] = (frames[fn] + best[0], [fn] + best[1])
    return memo[fn]


def short(name):
    return name.split(':')[-1]


def main():
    compile_all()
    frames, edges, dynamic = parse()
    by_short = {short(k): k for k in frames}
    notes = {'recursion': set(), 'external': set(), 'indirect': 0}
    lines = ['ARM9 STATIC STACK ESTIMATE (compiler frame sizes; not a runtime high-water measurement)',
             'Flags match the ROM build plus -fstack-usage -fcallgraph-info=su; objects in build/stack.']
    memo = {}
    for r in ROOTS:
        if r not in by_short:
            lines.append(f'{r}: not found')
            continue
        total, path = worst(by_short[r], frames, edges, set(), memo, notes)
        lines.append(f'{r}: worst known path {total} bytes ({total * 100 // DTCM_STACK}% of the 16 KiB DTCM stack region)')
        for f in path:
            lines.append(f'  {frames[f]:6d}  {short(f)}')
    lines.append('Largest single frames:')
    for f, n in sorted(frames.items(), key=lambda kv: -kv[1])[:12]:
        lines.append(f'  {n:6d}  {short(f)}' + ('  (dynamic)' if f in dynamic else ''))
    lines.append('Dynamic or bounded-dynamic frames: ' + (', '.join(sorted(short(f) for f in dynamic)) or 'none'))
    lines.append('Recursive functions (not followed): ' + (', '.join(sorted(short(f) for f in notes['recursion'])) or 'none'))
    lines.append(f'Indirect calls seen on searched paths: {notes["indirect"]} (not followed)')
    lines.append('SDK/libc callees counted as 0 bytes: ' + ', '.join(sorted(notes['external'])))
    text = '\n'.join(lines) + '\n'
    sys.stdout.write(text)
    if len(sys.argv) > 1:
        open(sys.argv[1], 'w', encoding='utf-8').write(text)


if __name__ == '__main__':
    main()
