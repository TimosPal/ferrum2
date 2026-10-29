$ErrorActionPreference = 'Stop'

# Git launched from an editor may not inherit the MSVC developer environment.
$commands = 'cmake --preset debug -DBUILD_TESTING=ON && cmake --build --preset debug --target ferrumCommonTests ferrumCoreTests && ctest --preset unit'
if (-not (Get-Command cl -ErrorAction SilentlyContinue)) {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path -LiteralPath $vswhere)) {
        throw 'pre-commit: MSVC environment unavailable and vswhere.exe was not found.'
    }
    $installation = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if ($LASTEXITCODE -ne 0 -or -not $installation) {
        throw 'pre-commit: no Visual Studio C++ toolchain was found.'
    }
    $setup = Join-Path $installation 'VC\Auxiliary\Build\vcvars64.bat'
    if (-not (Test-Path -LiteralPath $setup)) {
        throw "pre-commit: MSVC environment script not found: $setup"
    }
    $commands = "call `"$setup`" >nul && $commands"
}

& $env:ComSpec /d /c $commands
exit $LASTEXITCODE
