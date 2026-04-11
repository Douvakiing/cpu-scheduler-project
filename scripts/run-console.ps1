#Requires -Version 5.1
<#
  Compile main.cpp + every *.cpp under src/ (recursive) with a C++ compiler in PATH (no CMake) and run.
  Include path: src (for headers).

  Usage:
    .\scripts\run-console.ps1
    .\scripts\run-console.ps1 -SkipBuild    # run last build under out\console without recompiling

  Optional: set CXX to force a compiler (e.g. g++, clang++, cl).
#>
param(
  [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$Inc = Join-Path $Root "src"
$MainCpp = Join-Path $Root "main.cpp"
$OutDir = Join-Path $Root "out\console"
$IsWin = $env:OS -like "*Windows*"

if ($IsWin) {
  $OutExe = Join-Path $OutDir "cpu_scheduler.exe"
} else {
  $OutExe = Join-Path $OutDir "cpu_scheduler"
}

function Test-CompilerKind {
  param([string]$Path)
  $name = [System.IO.Path]::GetFileNameWithoutExtension($Path).ToLowerInvariant()
  if ($name -eq "cl") { return "msvc" }
  return "gnu"
}

function Invoke-GnuCompile {
  param([string]$Compiler, [string]$Out, [string]$Include, [string]$Main, [string[]]$SrcFiles)
  $compileArgs = @(
    "-std=c++17", "-O2",
    "-I", $Include,
    "-o", $Out,
    $Main
  ) + $SrcFiles
  & $Compiler @compileArgs
}

function Invoke-MsvcCompile {
  param([string]$Out, [string]$Include, [string]$Main, [string[]]$SrcFiles)
  Push-Location $Root
  try {
    $outDir = Split-Path -Parent $Out
    $fo = Join-Path $outDir ""
    if (-not $fo.EndsWith("\")) { $fo += "\" }
    $sources = @($Main) + $SrcFiles
    & cl /nologo /std:c++17 /EHsc "/Fe$Out" "/Fo$fo" "/I$Include" @sources
  } finally {
    Pop-Location
  }
}

function Get-DefaultCompiler {
  if ($env:CXX -and (Get-Command $env:CXX -ErrorAction SilentlyContinue)) {
    return $env:CXX
  }
  foreach ($c in @("g++", "clang++", "c++")) {
    if (Get-Command $c -ErrorAction SilentlyContinue) { return $c }
  }
  if ($IsWin -and (Get-Command cl -ErrorAction SilentlyContinue)) { return "cl" }
  return $null
}

if (-not (Test-Path -LiteralPath $MainCpp)) {
  Write-Error "Missing $MainCpp"
  exit 1
}
if (-not (Test-Path -LiteralPath $Inc -PathType Container)) {
  Write-Error "Missing src directory: $Inc"
  exit 1
}

$SrcCpps = @(
  Get-ChildItem -LiteralPath $Inc -Recurse -File -Filter "*.cpp" |
    Sort-Object FullName |
    ForEach-Object { $_.FullName }
)

if (-not $SkipBuild) {
  New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
  $compiler = Get-DefaultCompiler
  if (-not $compiler) {
    Write-Error "No C++ compiler found. Install g++/clang/MSVC or set CXX to the compiler executable."
    exit 1
  }

  $kind = Test-CompilerKind $compiler
  Write-Host "Compiling with: $compiler ($kind) ($($SrcCpps.Count) file(s) under src\)"

  if ($kind -eq "msvc") {
    Invoke-MsvcCompile -Out $OutExe -Include $Inc -Main $MainCpp -SrcFiles $SrcCpps
  } else {
    Invoke-GnuCompile -Compiler $compiler -Out $OutExe -Include $Inc -Main $MainCpp -SrcFiles $SrcCpps
  }
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

if (-not (Test-Path -LiteralPath $OutExe)) {
  Write-Error "No binary at $OutExe. Run without -SkipBuild to compile."
  exit 1
}

Write-Host "Running: $OutExe"
& $OutExe @args
exit $LASTEXITCODE
