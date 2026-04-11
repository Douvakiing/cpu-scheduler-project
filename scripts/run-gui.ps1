#Requires -Version 5.1
<#
  Build (optional) and run the GUI app (main_gui.cpp -> cpu_scheduler_gui).
  Usage:
    .\scripts\run-gui.ps1
    .\scripts\run-gui.ps1 -SkipBuild
    .\scripts\run-gui.ps1 -Config Debug
#>
param(
  [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel")]
  [string]$Config = "Release",
  [string]$BuildDir = "build",
  [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$absBuild = Join-Path $Root $BuildDir
$exeName = "cpu_scheduler_gui"

function Find-BuiltExecutable {
  param([string]$Name, [string]$BuildRoot, [string]$Cfg)
  $candidates = @(
    (Join-Path $BuildRoot "$Cfg\$Name.exe"),
    (Join-Path $BuildRoot "bin\$Cfg\$Name.exe"),
    (Join-Path $BuildRoot "console\$Cfg\$Name.exe"),
    (Join-Path $BuildRoot "$Name.exe"),
    (Join-Path $BuildRoot "$Cfg\$Name"),
    (Join-Path $BuildRoot "bin\$Cfg\$Name"),
    (Join-Path $BuildRoot "$Name")
  )
  foreach ($p in $candidates) {
    if (Test-Path -LiteralPath $p) { return (Resolve-Path -LiteralPath $p).Path }
  }
  return $null
}

if (-not $SkipBuild) {
  & (Join-Path $Root "scripts\build.ps1") -Config $Config -BuildDir $BuildDir
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

$exe = Find-BuiltExecutable -Name $exeName -BuildRoot $absBuild -Cfg $Config
if (-not $exe) {
  Write-Error "Could not find $exeName under $absBuild. Try a different -Config or build once from your IDE."
  exit 1
}

Write-Host "Running: $exe"
& $exe @args
exit $LASTEXITCODE
