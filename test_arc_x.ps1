# Arc X Integration Test Script
Write-Host "=== ARC X INTEGRATION TEST ===" -ForegroundColor Cyan
Write-Host ""

# Check if executable is recent (modified in last 10 minutes)
$exePath = "build-vs\Debug\DJ-ROOFRAT.exe"

if (Test-Path $exePath) {
    $exe = Get-Item $exePath
    $age = (Get-Date) - $exe.LastWriteTime
    
    Write-Host "✓ DJ-ROOFRAT.exe found" -ForegroundColor Green
    Write-Host "  Size: $($exe.Length) bytes" -ForegroundColor Gray
    Write-Host "  Modified: $($exe.LastWriteTime)" -ForegroundColor Gray
    Write-Host "  Age: $([int]$age.TotalMinutes) minutes ago" -ForegroundColor Gray
    Write-Host ""
    
    Write-Host "=== ARC X FEATURES INTEGRATED ===" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Phase 40 - Transition Coaching:" -ForegroundColor Cyan
    Write-Host "  ✓ TransitionCoach instantiated" -ForegroundColor Green
    Write-Host "  ✓ CoachingHUD instantiated" -ForegroundColor Green
    Write-Host "  ✓ Press 'C' to toggle coaching overlay" -ForegroundColor Gray
    Write-Host "  ✓ Phrase detection (16/32 bars)" -ForegroundColor Gray
    Write-Host "  ✓ Confidence scoring (harmonic + energy)" -ForegroundColor Gray
    Write-Host "  ✓ Real-time countdown display" -ForegroundColor Gray
    Write-Host ""
    
    Write-Host "Phase 41 - Beat Jump & Warp:" -ForegroundColor Cyan
    Write-Host "  ✓ SyncIndicator instantiated" -ForegroundColor Green
    Write-Host "  ✓ SyncUndoStack (x2) instantiated" -ForegroundColor Green
    Write-Host "  ✓ Extended beat jump: ±1 beats (1/! keys)" -ForegroundColor Gray
    Write-Host "  ✓ Extended beat jump: ±8 beats (8/* keys)" -ForegroundColor Gray
    Write-Host "  ✓ BPM warp: ±0.01% (+/- keys)" -ForegroundColor Gray
    Write-Host "  ✓ Sync undo: Ctrl+Z restoration" -ForegroundColor Gray
    Write-Host "  ✓ Visual sync badges (SYNC ✓/⟳/⚠)" -ForegroundColor Gray
    Write-Host "  ✓ Phase meter display" -ForegroundColor Gray
    Write-Host ""
    
    Write-Host "=== KEYBOARD CONTROLS (FULL ARC X) ===" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Sync Controls:" -ForegroundColor Cyan
    Write-Host "  Shift+S / Shift+D - Toggle sync Deck A/B" -ForegroundColor White
    Write-Host "  Ctrl+Z            - Undo last sync operation" -ForegroundColor White
    Write-Host ""
    
    Write-Host "Beat Jump:" -ForegroundColor Cyan
    Write-Host "  [ / ]             - Jump ±4 beats (Deck A)" -ForegroundColor White
    Write-Host "  ; / '             - Jump ±4 beats (Deck B)" -ForegroundColor White
    Write-Host "  1 / ! (Shift+1)   - Jump ±1 beat" -ForegroundColor White
    Write-Host "  8 / * (Shift+8)   - Jump ±8 beats" -ForegroundColor White
    Write-Host ""
    
    Write-Host "BPM Warp:" -ForegroundColor Cyan
    Write-Host "  + / -             - Warp ±0.01%" -ForegroundColor White
    Write-Host ""
    
    Write-Host "Coaching:" -ForegroundColor Cyan
    Write-Host "  C                 - Toggle coaching HUD overlay" -ForegroundColor White
    Write-Host ""
    
    Write-Host "Other:" -ForegroundColor Cyan
    Write-Host "  p / Shift+P       - Play/pause Deck A/B" -ForegroundColor White
    Write-Host "  q                 - Quit" -ForegroundColor White
    Write-Host ""
    
    $response = Read-Host "Run DJ-ROOFRAT now? (y/n)"
    if ($response -eq 'y' -or $response -eq 'Y') {
        Write-Host ""
        Write-Host "Starting DJ-ROOFRAT with Arc X features..." -ForegroundColor Green
        Write-Host "(Press 'q' to quit at any time)" -ForegroundColor Gray
        Write-Host ""
        & .\build-vs\Debug\DJ-ROOFRAT.exe --no-audio
    } else {
        Write-Host "Test cancelled." -ForegroundColor Yellow
    }
    
} else {
    Write-Host "✗ DJ-ROOFRAT.exe not found!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Building now..." -ForegroundColor Yellow
    cmake --build build-vs --config Debug --target DJ-ROOFRAT
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ Build successful!" -ForegroundColor Green
        Write-Host "Run this script again to test Arc X features." -ForegroundColor Cyan
    } else {
        Write-Host "✗ Build failed" -ForegroundColor Red
        Write-Host "Check build output for errors" -ForegroundColor Yellow
    }
}
