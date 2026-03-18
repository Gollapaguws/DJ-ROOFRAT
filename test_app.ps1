# Test DJ-ROOFRAT main app
Write-Host "=== DJ-ROOFRAT Test Session ===" -ForegroundColor Cyan
Write-Host ""

# Check if executable exists
if (Test-Path "build-vs\Debug\DJ-ROOFRAT.exe") {
    $exe = Get-Item "build-vs\Debug\DJ-ROOFRAT.exe"
    Write-Host "✓ Executable found: $($exe.Length) bytes, modified $($exe.LastWriteTime)" -ForegroundColor Green
    Write-Host ""
    
    Write-Host "Starting DJ-ROOFRAT with --no-audio flag..." -ForegroundColor Yellow
    Write-Host "Commands to test:" -ForegroundColor Cyan
    Write-Host "  - Press 'p' to play Deck A" -ForegroundColor Gray
    Write-Host "  - Press 'Shift+S' to toggle sync on Deck A" -ForegroundColor Gray
    Write-Host "  - Press '[' or ']' to beat jump ±4 beats" -ForegroundColor Gray
    Write-Host "  - Press '1' or '8' for ±1/±8 beat jumps" -ForegroundColor Gray
    Write-Host "  - Press '+' or '-' for warp ±0.01%" -ForegroundColor Gray
    Write-Host "  - Press 'C' to toggle coaching HUD" -ForegroundColor Gray
    Write-Host "  - Press 'q' to quit" -ForegroundColor Gray
    Write-Host ""
    
    & ".\build-vs\Debug\DJ-ROOFRAT.exe" --no-audio
    
} else {
    Write-Host "✗ Executable not found. Building..." -ForegroundColor Red
    cmake --build build-vs --config Debug --target DJ-ROOFRAT 2>&1 | Tee-Object -FilePath "build_log.txt"
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ Build successful!" -ForegroundColor Green
        Write-Host "Run this script again to test the app."
    } else {
        Write-Host "✗ Build failed. See build_log.txt for details." -ForegroundColor Red
        Get-Content "build_log.txt" | Select-Object -Last 30
    }
}
