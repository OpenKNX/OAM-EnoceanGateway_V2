"""Erzeugt include/EnoceanProfilsGenerated.h aus den EEP-Profillisten der ETS-Applikation.

Die Profilnummern (z.B. A5_09_04 = 43) sind die Werte, die die ETS in den Parametern speichert. Damit ETS und Firmware
nie auseinanderlaufen, werden sie nicht von Hand gepflegt, sondern aus den Enumerationen in
src/EnoceanModule.share.xml gelesen: Text "A5-09-04 ..." mit Value="43" ergibt "#define A5_09_04 43".
Ein "*" hinter dem Profil (z.B. "A5-06-01*") ergibt den Namen mit Endung _V2. Profile, die in der ETS mehrfach mit
verschiedenen Werten vorkommen (z.B. A5-38-08 Schalt-/Dimmaktor), werden ausgelassen und bleiben in EnoceanProfils.h.

Aufruf: als PlatformIO-Pre-Script (platformio.custom.ini) oder direkt: python scripts/generate_profiles.py
"""
import os
import re
import sys

PROFILE_TYPES = re.compile(r'^(Profil4BS\w*|ProfilVLD\w*|ProfilSelectionRPS|ProfilSelection1BS)$')


def generate(root):
    share = os.path.join(root, 'src', 'EnoceanModule.share.xml')
    target = os.path.join(root, 'include', 'EnoceanProfilsGenerated.h')
    xml = open(share, encoding='utf-8').read()

    values = {}
    for m in re.finditer(r'<ParameterType Id="%AID%_PT-(\w+)" Name="\w+">(.*?)</ParameterType>', xml, re.S):
        if not PROFILE_TYPES.match(m.group(1)):
            continue
        for e in re.finditer(r'<Enumeration Text="\s*([A-F0-9]{2})-([0-9A-F]{2})-([0-9A-F]{2})(\*?)[^"]*" Value="(\d+)"', m.group(2)):
            name = f'{e.group(1)}_{e.group(2)}_{e.group(3)}' + ('_V2' if e.group(4) else '')
            values.setdefault(name, set()).add(int(e.group(5)))

    lines = ['// Automatisch erzeugt von scripts/generate_profiles.py aus src/EnoceanModule.share.xml - nicht von Hand ändern!',
             '// Profilnummern = Werte der EEP-Profilauswahl in der ETS.',
             '#pragma once', '']
    skipped = []
    for name in sorted(values):
        if len(values[name]) > 1:
            skipped.append(name)
            continue
        lines.append(f'#define {name} {next(iter(values[name]))}')
    if skipped:
        lines += ['', '// mehrdeutig, siehe EnoceanProfils.h: ' + ', '.join(skipped)]
    content = '\n'.join(lines) + '\n'

    old = open(target, encoding='utf-8').read() if os.path.exists(target) else None
    if old != content:  # nur bei Änderung schreiben, sonst baut PlatformIO unnötig neu
        open(target, 'w', encoding='utf-8', newline='\n').write(content)
        print(f'generate_profiles: {target} aktualisiert ({len(values) - len(skipped)} Profile)')


try:
    Import('env')  # noqa: F821 - PlatformIO/SCons
    generate(env['PROJECT_DIR'])  # noqa: F821
except NameError:
    generate(os.path.dirname(os.path.dirname(os.path.abspath(sys.argv[0]))))
