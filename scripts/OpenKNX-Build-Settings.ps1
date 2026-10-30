# set product names, allows mapping of (devel) name in Project to a more consistent name in release
$settings = lib/OGM-Common/scripts/build/OpenKNX-Build-Settings.ps1 $args[0] "EnoceanGateway" "EnoceanGateway"

# there is only one main xml (src/EnoceanGateway.xml), use it for Beta and Release builds
$settings.releaseName = $settings.sourceName

Return $settings
