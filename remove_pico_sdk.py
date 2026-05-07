import shutil
from pathlib import Path
p = Path(r'G:/My Drive/projects/pico/pico-sdk')
if p.exists():
    shutil.rmtree(p)
    print('removed')
else:
    print('missing')
