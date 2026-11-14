# This script is just a template and has to be copied and modified per project
# This script should be called from .vscode/tasks.json with
#
#   scripts/Build-Release.ps1            - for Beta builds
#   scripts/Build-Release.ps1 Release    - for Release builds
#
# {
#     "label": "Build-Release",
#     "type": "shell",
#     "command": "scripts/Build-Release.ps1 Release",
#     "args": [],
#     "problemMatcher": [],
#     "group": "test"
# },
# {
#     "label": "Build-Beta",
#     "type": "shell",
#     "command": "scripts/Build-Release.ps1 ",
#     "args": [],
#     "problemMatcher": [],
#     "group": "test"
# }

# set product names, allows mapping of (devel) name in Project to a more consistent name in release
# $settings = scripts/OpenKNX-Build-Settings.ps1

# execute generic pre-build steps
lib/OGM-Common/scripts/setup/reusable/Build-Release-Preprocess.ps1 $args[0]
if (!$?) { exit 1 }

# EnOcean: erzeugte ETS-Applikation prüfen (when-Werte, Taster-KOs), siehe scripts/check_ets_ui.py
$python = "$env:USERPROFILE/.platformio/penv/Scripts/python.exe"
if (!(Test-Path $python)) { $python = "python" }
& $python scripts/check_ets_ui.py
if ($LASTEXITCODE -ne 0) {
  Write-Host "ETS-Applikation fehlerhaft, Release abgebrochen (scripts/check_ets_ui.py)" -ForegroundColor Red
  exit 1
}

# build firmware for SmartMF EnOcean-Gateway V2 (RP2040, KNX TP)
lib/OGM-Common/scripts/setup/reusable/Build-Step.ps1 release_RP2040_TP firmware-EnoceanGateway-V2 rp2040-tp SmartMF-EnoceanGateway-V2
if (!$?) { exit 1 }

# execute generic post-build steps
lib/OGM-Common/scripts/setup/reusable/Build-Release-Postprocess.ps1 $args[0]
if (!$?) { exit 1 }
