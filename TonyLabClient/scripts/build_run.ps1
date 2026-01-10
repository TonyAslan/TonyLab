# TonyLabClient one-click configure/build/run script (PowerShell)
# Usage examples:
#   .\scripts\build_run.ps1
#   .\scripts\build_run.ps1 -Config Release
#   .\scripts\build_run.ps1 -Clean -Reconfigure
#   .\scripts\build_run.ps1 -RunArgs @('--some-flag','123')

[CmdletBinding()]
param(
    # CMake build directory (默认：<project>/build)
    [string]$BuildDir = '',

    # CMake configuration for multi-config generators (Debug/Release/RelWithDebInfo/MinSizeRel)
    [ValidateSet('Debug','Release','RelWithDebInfo','MinSizeRel')]
    [string]$Config = 'Debug',

    # Optional: explicitly set a CMake generator (e.g. "Visual Studio 17 2022", "Ninja")
    [string]$Generator = '',

    # Re-run CMake configure step even if already configured
    [switch]$Reconfigure,

    # Delete the build directory before configuring
    [switch]$Clean,

    # Build in parallel (CMake will pick a sensible default when omitted)
    [int]$Parallel = 0,

    # Build 完成后是否运行可执行文件
        # 不运行程序（只配置/编译）。PowerShell 原生 switch 参数，跨终端最稳。
        [switch]$NoRun,

    # Arguments passed to the executable
    [string[]]$RunArgs = @()
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Resolve-ProjectRoot {
    # scripts/ -> project root
    $root = Resolve-Path (Join-Path $PSScriptRoot '..')
    return $root.Path
}

function Ensure-CMake {
    if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
        throw 'cmake not found in PATH. Install CMake and ensure it is available from the terminal.'
    }
}

function Clean-BuildDir([string]$dir) {
    if (Test-Path $dir) {
        Write-Host "[clean] Removing build dir: $dir"
        Remove-Item -LiteralPath $dir -Recurse -Force
    }
}

function Configure([string]$srcDir, [string]$dir) {
    Write-Host "[cmake] Configure: $srcDir -> $dir"
    $args = @('-S', $srcDir, '-B', $dir)
    if ($Generator -and $Generator.Trim().Length -gt 0) {
        $args += @('-G', $Generator)
    }
    & cmake @args
}

function Build([string]$dir) {
    Write-Host "[cmake] Build: $dir (config=$Config)"
    $args = @('--build', $dir, '--config', $Config)
    if ($Parallel -gt 0) {
        $args += @('--parallel', $Parallel)
    }
    & cmake @args
}

function Resolve-ExePath([string]$dir) {
    # Multi-config (MSVC): <build>/bin/<Config>/TonyLabClient.exe
    $candidate1 = Join-Path $dir (Join-Path 'bin' (Join-Path $Config 'TonyLabClient.exe'))
    # Single-config (Ninja/Make): <build>/bin/TonyLabClient.exe
    $candidate2 = Join-Path $dir (Join-Path 'bin' 'TonyLabClient.exe')

    if (Test-Path $candidate1) { return (Resolve-Path $candidate1).Path }
    if (Test-Path $candidate2) { return (Resolve-Path $candidate2).Path }

    # Fallback: search within bin
    $binDir = Join-Path $dir 'bin'
    if (Test-Path $binDir) {
        $found = Get-ChildItem -Path $binDir -Filter 'TonyLabClient.exe' -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($found) { return $found.FullName }
    }

    throw "TonyLabClient.exe not found under build output. Looked for: $candidate1 and $candidate2"
}

function Run-Exe([string]$exe) {
    Write-Host "[run] $exe $($RunArgs -join ' ')"
    & $exe @RunArgs
    $exit = $LASTEXITCODE
    if ($exit -ne 0) {
        throw "Program exited with code $exit"
    }
}

# --- main ---
Ensure-CMake
$projectRoot = Resolve-ProjectRoot
$buildDirInput = $BuildDir
if ([string]::IsNullOrWhiteSpace($buildDirInput)) {
    $buildDirInput = (Join-Path $projectRoot 'build')
}
$buildDirFull = Resolve-Path -Path $buildDirInput -ErrorAction SilentlyContinue
if (-not $buildDirFull) {
    # Build dir may not exist yet
    $buildDirFull = (Resolve-Path -Path (Join-Path $projectRoot 'build') -ErrorAction SilentlyContinue)
    if (-not $buildDirFull) {
        $buildDirFull = [System.IO.Path]::GetFullPath((Join-Path $projectRoot 'build'))
    } else {
        $buildDirFull = $buildDirFull.Path
    }
} else {
    $buildDirFull = $buildDirFull.Path
}

if ($Clean) {
    Clean-BuildDir $buildDirFull
}

if ($Reconfigure -or -not (Test-Path (Join-Path $buildDirFull 'CMakeCache.txt'))) {
    Configure $projectRoot $buildDirFull
}

Build $buildDirFull

if (-not $NoRun) {
    $exe = Resolve-ExePath $buildDirFull
    Run-Exe $exe
}

Write-Host '[done] ok'
