"""Prüft die erzeugte ETS-Applikation (src/EnoceanGateway.debug.xml, entsteht beim OpenKNXproducer-Lauf mit --Debug).

1. Jeder Wert in <when test="..."> eines EnOcean-Kanals ist ein gültiger Wert des Parametertyps (sonst wird der Zweig
   nie angezeigt, z.B. FHD60SB hing an Wert 0 statt 1).
2. Für zufällige Taster-Konfigurationen (EEP-Rocker und alle Eltako-Taster, jede Bedienung und Tastenfunktion):
   - keine zwei sichtbaren KO-Verknüpfungen auf dasselbe KO,
   - kein Parameter doppelt sichtbar (außer bewusst doppelten wie Kanalname und Teach-Status in der Übersicht),
   - jede sichtbare Einzeltaste hat ihr erstes KO und, wenn benötigt, ihr zweites KO (und sonst keins).

Aufruf: python scripts/check_ets_ui.py [Anzahl Konfigurationen]   -> Exit-Code 1 bei Fehlern
"""
import os
import random
import re
import sys
import xml.etree.ElementTree as ET

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
NS = '{http://knx.org/xml/project/20}'
RUNS = int(sys.argv[1]) if len(sys.argv) > 1 else 20000

# bewusst mehrfach sichtbar: Kanalname (053) und Teach-Status (096) stehen auch in der Übersichtstabelle
ALLOWED_DUPLICATE_PARAMS = ('053', '096')

# Einzeltasten: Funktion, langer Tastendruck, erstes und zweites KO (Index im Kanal)
KEYS = {'AO': (0, 1), 'AU': (2, 3), 'BO': (4, 5), 'BU': (6, 7), 'CO': (8, 9), 'CU': (10, 11)}

app = ET.parse(os.path.join(ROOT, 'src', 'EnoceanGateway.debug.xml')).getroot()
knxprod = open(os.path.join(ROOT, 'include', 'knxprod.h'), encoding='utf-8').read()
ko_offset = int(re.search(r'#define ENO_KoOffset (\d+)', knxprod).group(1))

ptypes = {pt.get('Id'): [int(e.get('Value')) for e in pt.iter(NS + 'Enumeration')] for pt in app.iter(NS + 'ParameterType')}
params = {p.get('Id'): p for p in app.iter(NS + 'Parameter')}
pname = {pid: p.get('Name') for pid, p in params.items()}
prefs = {r.get('Id'): r.get('RefId') for r in app.iter(NS + 'ParameterRef')}
korefs = {r.get('Id'): r.get('RefId') for r in app.iter(NS + 'ComObjectRef')}
errors = set()


def default(pid):
    return int(params[pid].get('Value') or 0)


def match(test, v):
    for t in test.split():
        m = re.fullmatch(r'(!=|<=|>=|<|>|=)?(-?\d+)', t)
        if m:
            op, n = m.group(1) or '=', int(m.group(2))
            if {'=': v == n, '!=': v != n, '<': v < n, '>': v > n, '<=': v <= n, '>=': v >= n}[op]:
                return True
    return False


# 1) when-Werte gegen Parametertypen (nur EnOcean-Kanal 1)
for ch in app.iter(NS + 'choose'):
    pid = prefs[ch.get('ParamRefId')]
    if not pname[pid].startswith('ENO_CH1') or pname[pid][7:8].isdigit():
        continue
    vals = ptypes.get(params[pid].get('ParameterType'))
    if not vals:
        continue
    for w in ch.findall(NS + 'when'):
        for t in (w.get('test') or '').split():
            if re.fullmatch(r'-?\d+', t) and int(t) not in vals:
                errors.add(f'when test="{w.get("test")}" bei {pname[pid]}: Wert {t} gibt es im Parametertyp nicht')

# Oberfläche von Kanal 1
ch1_param = next(pid for pid, n in pname.items() if n == 'ENO_CH1Productsel')
ch1_tag = re.search(r'-(\d+)$', ch1_param).group(1)[:-3]  # z.B. "2001"
dynamic = next(app.iter(NS + 'Dynamic'))
channel = next(c for c in dynamic.iter(NS + 'Channel')
               if any(f'-{ch1_tag}' in (r.get('RefId') or '') for r in c.iter(NS + 'ParameterRefRef')))


def walk(el, values, vis_p, vis_k):
    for c in el:
        tag = c.tag.replace(NS, '')
        if tag == 'choose':
            pid = prefs[c.get('ParamRefId')]
            v = values.get(pname[pid], default(pid))
            for w in c.findall(NS + 'when'):
                t = w.get('test')
                if (t is not None and match(t, v)) or (t is None and w.get('default') == 'true'):
                    walk(w, values, vis_p, vis_k)
        elif tag == 'ParameterRefRef':
            vis_p.append(c.get('RefId'))
        elif tag == 'ComObjectRefRef':
            vis_k.append(c.get('RefId'))
        else:
            walk(c, values, vis_p, vis_k)


def ko_index(ref):
    return int(korefs[ref].split('_O-')[1]) - ko_offset


tas_type = next(k for k in ptypes if k.endswith('_PT-ProductSelectionTypeTaster'))
tas_vals = [v for v in ptypes[tas_type] if v]
random.seed(1)
for _ in range(RUNS):
    if random.random() < 0.5:
        v = dict(Productsel=1, ProfilSelection=250, DirectionKnxEnocean=0, RockerProfil=random.choice([1, 2, 3, 4]))
    else:
        v = dict(Productsel=2, ProductselType=1, ProductselTypeTas=random.choice(tas_vals))
    for m in 'ABC':
        v[f'RockerMode{m}'] = random.choice([0, 1])
        v[f'RockerFunktion{m}'] = random.randint(1, 7)
    for k in KEYS:
        v[f'RockerKey{k}Funktion'] = random.choice([1, 2, 3, 4, 5])
        v[f'RockerKey{k}Long'] = random.choice([0, 1, 2, 3, 4])
    v = {f'ENO_CH1{k}': val for k, val in v.items()}
    vp, vk = [], []
    walk(channel, v, vp, vk)
    # nur Kanal 1 (die Kanalübersicht zeigt auch Parameter der anderen Kanäle)
    vp = [r for r in vp if f'_R-{ch1_tag}' in r]
    vk = [r for r in vk if f'_R-{ch1_tag}' in r]
    kos = [ko_index(r) for r in vk]
    dup = {k for k in kos if kos.count(k) > 1}
    if dup:
        errors.add(f'KO doppelt sichtbar {sorted(dup)} bei Produktsel {v["ENO_CH1Productsel"]}')
    dp = {p for p in vp if vp.count(p) > 1 and not any(f'-{ch1_tag}{n}_' in p for n in ALLOWED_DUPLICATE_PARAMS)}
    if dp:
        errors.add(f'Parameter doppelt sichtbar {sorted(dp)}')
    visible = {pname[prefs[p]] for p in vp}
    for k, (o1, o2) in KEYS.items():
        if f'ENO_CH1RockerKey{k}Funktion' not in visible:
            continue
        f, lng = v[f'ENO_CH1RockerKey{k}Funktion'], v[f'ENO_CH1RockerKey{k}Long']
        need2 = f in (4, 5) or lng != 0
        if o1 not in kos:
            errors.add(f'Taste {k}: erstes KO {o1} fehlt (Funktion {f})')
        if need2 and o2 not in kos:
            errors.add(f'Taste {k}: zweites KO {o2} fehlt (Funktion {f}, lang {lng})')
        if not need2 and o2 in kos:
            errors.add(f'Taste {k}: zweites KO {o2} sichtbar, obwohl nicht benötigt (Funktion {f})')

print(f'check_ets_ui: {RUNS} Taster-Konfigurationen geprüft, {len(errors)} Fehler')
for e in sorted(errors):
    print('  -', e)
sys.exit(1 if errors else 0)
