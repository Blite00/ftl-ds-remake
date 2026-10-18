#!/usr/bin/env python3
"""Read local species data and one original walk frame per base-game species."""
import hashlib
import io
import json
from pathlib import Path
from PIL import Image
from import_ftl import PkgArchive, xml_root
from convert_ship_art import c_bitmap
SPECIES = ('human','engi','mantis','rock','slug','energy','crystal')

def import_species(archive_path, output):
    archive_path, output = Path(archive_path).resolve(), Path(output).resolve()
    if output == archive_path.parent or archive_path.is_relative_to(output):
        raise ValueError('Output must be separate from installed archive')
    archive = PkgArchive(archive_path)
    files = []
    def original(name):
        payload = archive.read(name)
        path = output/'original'/name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(payload)
        if path.read_bytes() != payload:
            raise IOError('Extraction verification failed: ' + name)
        files.append(dict(path=name,size=len(payload),sha256=hashlib.sha256(payload).hexdigest()))
        return payload
    animations = xml_root(original('data/animations.xml'))
    blueprints = xml_root(original('data/blueprints.xml'))
    strings = {e.get('name'):e.text for e in xml_root(original('data/text_blueprints.xml')).iter('text')}
    header = ['/* Local original species sprites; do not redistribute. */', '#ifndef SPECIES_ART_H',
              '#define SPECIES_ART_H', '#include <stdint.h>', '#define SPECIES_ART_WIDTH 14', '#define SPECIES_ART_HEIGHT 14']
    metadata=[]
    preview=Image.new('RGBA',(14*len(SPECIES),14))
    for index,species in enumerate(SPECIES):
        sheet=animations.find(".//animSheet[@name='%s']"%species)
        anim=animations.find(".//anim[@name='%s_walk_down']/desc"%species)
        if sheet is None or anim is None:raise ValueError('Missing species animation '+species)
        fw,fh=int(sheet.get('fw')),int(sheet.get('fh'))
        x,y=int(anim.get('x'))*fw,int(anim.get('y'))*fh
        source=Image.open(io.BytesIO(original('img/'+sheet.text))).convert('RGBA')
        if x<0 or y<0 or x+fw>source.width or y+fh>source.height:raise ValueError('Invalid source frame')
        image=source.crop((x,y,x+fw,y+fh)).resize((14,14),Image.Resampling.NEAREST)
        header+=c_bitmap('species_'+species+'_art',image)
        image.save(output/('species_'+species+'.png'))
        preview.alpha_composite(image,(index*14,0))
        crew=blueprints.find(".//crewBlueprint[@name='%s']"%species)
        metadata.append(dict(id=species,title=strings[crew.find('title').get('id')],cost=int(crew.findtext('cost')),
                             rarity=int(crew.findtext('rarity')),power_descriptions=[strings[p.get('id')] for p in crew.findall('powerList/power')],
                             source='img/'+sheet.text,animation=species+'_walk_down',frame=0,rectangle=[x,y,x+fw,y+fh]))
    header+=['static const uint16_t *const species_art[7] = {'+','.join('species_'+s+'_art' for s in SPECIES)+'};','#endif','']
    (output/'species_art.h').write_text('\n'.join(header),encoding='utf-8')
    (output/'species_audit.json').write_text(json.dumps(dict(species=metadata,sources=files,
        note='Installed original text is source prose, not engine numeric proof. Post-AE mentions remain audit data and do not enable AE features. One walk frame only; tint layers and full animations not reconstructed.',
        conversion='14x14 nearest-neighbor RGB555; alpha below96 transparent; energy maps to Zoltan'),indent=2),encoding='utf-8')
    preview.resize((392,56),Image.Resampling.NEAREST).save(output/'species_preview.png')
    print('Imported 7 original species frames: 2744 RGB555 bytes; base-game species only.')
