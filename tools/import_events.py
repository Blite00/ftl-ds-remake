#!/usr/bin/env python3
"""Import one original, effect-free quiet beacon from the owner's local FTL.

This selects a verified text-only event; it is not an FTL event interpreter.
Generated game text is local data and must stay out of source releases.
"""
import argparse
import hashlib
import json
from pathlib import Path
from import_ftl import PkgArchive, xml_root


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('archive', type=Path, nargs='?')
    ap.add_argument('--output', type=Path, default=Path(__file__).resolve().parents[1] / 'generated')
    args = ap.parse_args()
    output = args.output.resolve()
    archive_path = args.archive or Path(json.loads((output / 'manifest.json').read_text())['archive'])
    if output == archive_path.resolve().parent or archive_path.resolve().is_relative_to(output):
        raise ValueError('Output must be separate from the installed archive')
    archive = PkgArchive(archive_path)
    payloads = {name:archive.read(name) for name in ('data/events.xml','data/text_events.xml')}
    events = xml_root(payloads['data/events.xml'])
    event = events.find(".//event[@name='NOTHING']")
    if event is None or len(event) != 1 or event[0].tag != 'text' or event[0].get('load') != 'NOTHING':
        raise ValueError('NOTHING is no longer an effect-free text-list event')
    selected_id = 'text_NOTHING_2'
    selected = events.find(".//textList[@name='NOTHING']/text[@id='%s']" % selected_id)
    strings = xml_root(payloads['data/text_events.xml'])
    text_node = strings.find(".//text[@name='%s']" % selected_id)
    if selected is None or text_node is None or not text_node.text:
        raise ValueError('Original quiet beacon text missing')
    text = text_node.text
    if len(text) > 220 or not text.isascii():
        raise ValueError('Quiet text needs explicit DS display adaptation')
    header = ['/* Original local FTL text. Do not redistribute. Text-only selection, not an event interpreter. */',
              '#ifndef SECTOR_EVENTS_H', '#define SECTOR_EVENTS_H',
              'static const char sector_quiet_text[] = %s;' % json.dumps(text), '#endif', '']
    data = dict(archive=str(archive_path.resolve()),
                scope='One fixed original quiet-beacon text. Prototype map, supplies and repair remain reconstructed.',
                selections=[dict(symbol='sector_quiet_text',event_file='data/events.xml',event_id='NOTHING',
                                 text_list='NOTHING',text_id=selected_id,text_file='data/text_events.xml',
                                 text=text,text_sha256=hashlib.sha256(text.encode('utf-8')).hexdigest(),effects=[])],
                omitted=dict(supply='No exact standalone +2 fuel/+8 scrap match identified in base event files.',
                             repair='SAVE_CIVILIAN_LIST child 5 repairs 5 hull but requires a rescued-civilian context; not used for a generic repair node.'),
                source_files=[dict(path=name,size=len(payload),sha256=hashlib.sha256(payload).hexdigest())
                              for name,payload in payloads.items()])
    output.mkdir(parents=True,exist_ok=True)
    (output / 'sector_events.h').write_text('\n'.join(header),encoding='utf-8')
    (output / 'sector_events.json').write_text(json.dumps(data,indent=2),encoding='utf-8')
    print('Imported NOTHING/text_NOTHING_2: %d characters; no gameplay effects' % len(text))


if __name__ == '__main__':
    main()
