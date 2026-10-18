#!/usr/bin/env python3
"""Read FTL 1.6 PKG assets and produce a small local DS data set (stdlib only).

Never opens the source archive for writing. Generated original content is local
game data: do not include generated/ in a source release.
"""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import struct
import xml.etree.ElementTree as ET
import zlib


class PkgArchive:
    def __init__(self, path):
        self.path = Path(path)
        self.size = self.path.stat().st_size
        self.entries = {}
        with self.path.open('rb') as f:
            magic, header_size, entry_size, count, names_size = struct.unpack('>4sHHII', f.read(16))
            if magic != b'PKG\n' or header_size != 16 or entry_size != 20:
                raise ValueError('Expected FTL 1.6 PKG archive (16-byte header, 20-byte entries)')
            if 16 + count * 20 + names_size > self.size:
                raise ValueError('Archive index exceeds file size')
            index = f.read(count * entry_size)
            names = f.read(names_size)
            for i in range(count):
                _, name_flags, offset, size, unpacked = struct.unpack_from('>IIIII', index, i * 20)
                name_offset = name_flags & 0xffffff
                flags = name_flags >> 24
                if name_offset >= names_size or offset + size > self.size:
                    raise ValueError('Invalid archive entry bounds')
                end = names.find(b'\0', name_offset)
                if end < 0:
                    raise ValueError('Unterminated archive name')
                name = names[name_offset:end].decode('utf-8')
                safe = PurePosixPath(name)
                if safe.is_absolute() or '..' in safe.parts or '\\' in name or ':' in name:
                    raise ValueError('Unsafe archive path')
                if name in self.entries:
                    raise ValueError('Duplicate archive path: ' + name)
                self.entries[name] = (offset, size, unpacked, flags)

    def read(self, name):
        offset, size, unpacked, flags = self.entries[name]
        with self.path.open('rb') as f:
            f.seek(offset)
            data = f.read(size)
        if flags & ~1:
            raise ValueError('Unsupported PKG flags')
        if flags & 1:
            data = zlib.decompress(data)
        if len(data) != unpacked:
            raise ValueError('Incorrect extracted size: ' + name)
        return data


def xml_root(data):
    text = data.decode('utf-8-sig')
    text = re.sub(r'<!--.*?-->', '', text, flags=re.S)
    text = re.sub(r'<\?xml.*?\?>', '', text, flags=re.S)
    return ET.fromstring('<root>' + text + '</root>')


def layout(data):
    tokens = data.decode().split()
    fields = {'X_OFFSET': 1, 'Y_OFFSET': 1, 'VERTICAL': 1, 'HORIZONTAL': 1, 'ELLIPSE': 4, 'ROOM': 5, 'DOOR': 5}
    result = {'rooms': [], 'doors': []}
    i = 0
    while i < len(tokens):
        key = tokens[i]
        count = fields[key]
        values = list(map(int, tokens[i + 1:i + 1 + count]))
        if len(values) != count:
            raise ValueError('Truncated layout')
        if key == 'ROOM':
            result['rooms'].append(dict(zip(('id', 'x', 'y', 'w', 'h'), values)))
        elif key == 'DOOR':
            result['doors'].append(dict(zip(('x', 'y', 'room_a', 'room_b', 'vertical'), values)))
        else:
            result[key.lower()] = values
        i += count + 1
    return result


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('archive', type=Path)
    ap.add_argument('--output', type=Path, default=Path(__file__).resolve().parents[1] / 'generated')
    args = ap.parse_args()
    archive = PkgArchive(args.archive)
    output = args.output.resolve()
    if output == args.archive.resolve().parent or args.archive.resolve().is_relative_to(output):
        raise ValueError('Output must be separate from the installed archive')
    output.mkdir(parents=True, exist_ok=True)
    selected = ['data/blueprints.xml', 'data/text_blueprints.xml', 'data/kestral.txt', 'data/kestral.xml',
                'img/ship/kestral_base.png', 'img/ship/kestral_floor.png']
    extracted = {}
    manifest = []
    for name in selected:
        data = archive.read(name)
        extracted[name] = data
        dest = output / 'original' / name
        dest.parent.mkdir(parents=True, exist_ok=True)
        dest.write_bytes(data)
        # One integrity pass proves the extracted file matches the archive payload.
        if dest.read_bytes() != data:
            raise IOError('Extraction verification failed: ' + name)
        manifest.append({'path': name, 'size': len(data), 'sha256': hashlib.sha256(data).hexdigest()})
    root = xml_root(extracted['data/blueprints.xml'])
    ship = root.find(".//shipBlueprint[@name='PLAYER_SHIP_HARD']")
    if ship is None or ship.attrib['layout'] != 'kestral':
        raise ValueError('Standard Kestrel blueprint not found')
    text = xml_root(extracted['data/text_blueprints.xml'])
    strings = {e.attrib['name']: e.text for e in text.iter('text') if 'name' in e.attrib}
    data = layout(extracted['data/kestral.txt'])
    data.update(blueprint=ship.attrib['name'], hull=int(ship.find('health').attrib['amount']),
                reactor=int(ship.find('maxPower').attrib['amount']),
                crew_count=int(ship.find('crewCount').attrib['amount']),
                crew_species=ship.find('crewCount').attrib['class'],
                missiles=int(ship.find('weaponList').attrib['missiles']))
    data['systems'] = [dict(name=s.tag, room=int(s.attrib['room']), power=int(s.attrib['power']),
                            start=int(s.attrib.get('start') == 'true')) for s in ship.find('systemList')]
    data['weapons'] = []
    for entry in ship.find('weaponList'):
        name = entry.attrib['name']
        weapon = root.find(".//weaponBlueprint[@name='%s']" % name)
        title = weapon.find('title')
        item = dict(id=name, name=title.text or strings.get(title.attrib.get('id'), name))
        item.update(cooldown_ms=round(float(weapon.findtext('cooldown')) * 1000))
        for key, tag in [('damage', 'damage'), ('shots', 'shots'), ('power', 'power'),
                         ('missiles', 'missiles'), ('shield_piercing', 'sp')]:
            item[key] = int(weapon.findtext(tag, '0'))
        data['weapons'].append(item)
    header = ['/* Generated from the owner\'s FTL archive; do not redistribute. */',
              '#ifndef FTL_DATA_H', '#define FTL_DATA_H',
              'typedef struct { int id,x,y,w,h; } FtlRoom;',
              'typedef struct { int x,y,room_a,room_b,vertical; } FtlDoor;',
              'typedef struct { const char *name; int room,power,start; } FtlSystem;',
              'typedef struct { const char *id,*name; int cooldown_ms,damage,shots,power,missiles,shield_piercing; } FtlWeapon;']
    for key in ('hull', 'reactor', 'crew_count', 'missiles'):
        header.append('#define KESTREL_%s %d' % (key.upper(), data[key]))
    for key, typ, fields in [('rooms', 'FtlRoom', ('id','x','y','w','h')),
                              ('doors', 'FtlDoor', ('x','y','room_a','room_b','vertical')),
                              ('systems', 'FtlSystem', ('name','room','power','start')),
                              ('weapons', 'FtlWeapon', ('id','name','cooldown_ms','damage','shots','power','missiles','shield_piercing'))]:
        header.append('#define KESTREL_%s_COUNT %d' % (key[:-1].upper(), len(data[key])))
        header.append('static const %s kestrel_%s[] = {' % (typ, key))
        header.extend('    {' + ','.join(json.dumps(item[f]) for f in fields) + '},' for item in data[key])
        header.append('};')
    header.append('#endif\n')
    (output / 'ftl_data.h').write_text('\n'.join(header), encoding='utf-8')
    (output / 'kestrel.json').write_text(json.dumps(data, indent=2), encoding='utf-8')
    (output / 'manifest.json').write_text(json.dumps({'archive': str(args.archive.resolve()),
                'archive_entries': len(archive.entries), 'files': manifest}, indent=2), encoding='utf-8')
    print('Imported and verified %d files; %d rooms, %d doors, %d weapons -> %s' %
          (len(selected), len(data['rooms']), len(data['doors']), len(data['weapons']), output))


if __name__ == '__main__':
    main()
