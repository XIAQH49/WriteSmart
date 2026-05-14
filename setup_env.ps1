$ErrorActionPreference = "Continue"
$PROJECT_ROOT = Split-Path -Parent $MyInvocation.MyCommand.Path
$VSC_DIR = Join-Path $PROJECT_ROOT ".vscode"

Write-Host "WriteSmart 2.0 环境配置" -ForegroundColor Cyan

Write-Host ">>> 安装 .vscode 配置文件 ..." -ForegroundColor Yellow
if (!(Test-Path $VSC_DIR)) {
    New-Item -ItemType Directory -Path $VSC_DIR -Force | Out-Null
}

Copy-Item (Join-Path $PROJECT_ROOT ".vscode_cmake-kits.json")       (Join-Path $VSC_DIR "cmake-kits.json")       -Force
Copy-Item (Join-Path $PROJECT_ROOT ".vscode_c_cpp_properties.json") (Join-Path $VSC_DIR "c_cpp_properties.json") -Force
Copy-Item (Join-Path $PROJECT_ROOT ".vscode_settings.json")         (Join-Path $VSC_DIR "settings.json")         -Force
Write-Host "  [OK] 3 个配置文件已安装" -ForegroundColor Green

Write-Host ">>> 生成 activate.ps1 ..." -ForegroundColor Yellow
$actPath = Join-Path $PROJECT_ROOT "activate.ps1"
$actLines = @(
    '$env:PATH = "D:\Qt\6.9.1\mingw_64\bin;D:\Qt\Tools\CMake_64\bin;D:\MinGW-w64\mingw64\bin;$env:PATH"',
    'Write-Host "[WriteSmart] Qt + MinGW + CMake - activated" -ForegroundColor Green'
)
$actLines | Out-File -FilePath $actPath -Encoding utf8
Write-Host "  [OK] activate.ps1" -ForegroundColor Green

Write-Host ">>> 写入 PowerShell Profile ..." -ForegroundColor Yellow
$profilePath = $PROFILE.CurrentUserAllHosts
if (!$profilePath) {
    $profilePath = Join-Path $env:USERPROFILE "Documents\PowerShell\Microsoft.PowerShell_profile.ps1"
}
$profileDir = Split-Path $profilePath -Parent
if (!(Test-Path $profileDir)) {
    New-Item -ItemType Directory -Path $profileDir -Force | Out-Null
}
$activateLine = ". '$actPath'"
$existing = if (Test-Path $profilePath) { Get-Content $profilePath -Raw } else { "" }
if ($existing -notmatch [regex]::Escape($activateLine)) {
    $newContent = @"
# WriteSmart env activation
$activateLine
"@
    if ($existing.Length -gt 0) { $newContent = $existing + "`n`n" + $newContent }
    $newContent | Out-File -FilePath $profilePath -Encoding utf8
    Write-Host "  [OK] Profile updated" -ForegroundColor Green
} else {
    Write-Host "  [OK] Profile already configured" -ForegroundColor Green
}

Write-Host ">>> 激活环境 ..." -ForegroundColor Yellow
$env:PATH = "D:\Qt\6.9.1\mingw_64\bin;D:\Qt\Tools\CMake_64\bin;D:\MinGW-w64\mingw64\bin;$env:PATH"
Write-Host "  [OK] Done" -ForegroundColor Green

Write-Host ""
Write-Host "All done! Restart IDE for IntelliSense to take effect." -ForegroundColor Green
