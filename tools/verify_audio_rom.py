"""Verify imported audio payloads inside the built ROM's actual NitroFS."""
from pathlib import Path
import hashlib,json,struct
r=Path(__file__).resolve().parents[1]
rom=(r/'ftl-ds.nds').read_bytes();fnt,size,fat,fatsize=struct.unpack_from('<4I',rom,0x40)
assert fnt and fat and size and fatsize
files={}
def directory(index,prefix):
 offset,file_id,parent=struct.unpack_from('<IHH',rom,fnt+index*8)
 cursor=fnt+offset
 while True:
  tag=rom[cursor];cursor+=1
  if not tag:break
  name=rom[cursor:cursor+(tag&127)].decode('ascii');cursor+=tag&127
  if tag&128:
   child=struct.unpack_from('<H',rom,cursor)[0];cursor+=2;directory(child&4095,prefix+name+'/')
  else:
   a,b=struct.unpack_from('<II',rom,fat+file_id*8);file_id+=1
   assert 0<=a<=b<=len(rom)
   files[prefix+name]=rom[a:b]
directory(0,'')
manifest=json.loads((r/'generated/audio_manifest.json').read_text())
for asset in manifest['assets']:
 path=asset['nitro_path'].removeprefix('nitro:/');data=files[path]
 assert len(data)==asset['bytes'] and hashlib.sha256(data).hexdigest()==asset['sha256'],path
print(f"Verified {len(manifest['assets'])} audio files ({manifest['totals']['rom_asset_bytes']:,} bytes) embedded byte-exact in ROM NitroFS.")
