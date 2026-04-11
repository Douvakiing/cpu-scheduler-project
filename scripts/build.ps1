#Requires -Version 5.1
<#
  Configure and build with CMake. Works with Visual Studio, Ninja, MinGW, etc.
  Usage:
    .\scripts\build.ps1
    .\scripts\build.ps1 -Config Debug
    .\scripts\build.ps1 -BuildDir out\cmake-build
#>
param(
  [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel")]
  [string]$Config = "Release",
  [string]$BuildDir = "build",
  [string[]]$ExtraCmakeArgs = @()
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Set-Location $Root

$absBuild = Join-Path $Root $BuildDir
$configureArgs = @("-B", $absBuild, "-S", $Root) + $ExtraCmakeArgs
& cmake @configureArgs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$buildArgs = @("--build", $absBuild, "--parallel")
if ($Config) { $buildArgs += @("--config", $Config) }
& cmake @buildArgs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Build finished: $absBuild ($Config)"
