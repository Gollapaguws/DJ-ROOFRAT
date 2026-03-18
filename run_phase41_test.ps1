# Build and test Phase 41
Write-Host "Building Phase 41 test..."
cmake --build build-vs --config Debug --target beatjump_phase41_test 2>&1 | Out-File -FilePath "phase41_build.txt"

if ($LASTEXITCODE -eq 0) {
    Write-Host "Build successful, running tests..."
    & .\build-vs\Debug\BeatJump_Phase41_test.exe 2>&1 | Out-File -FilePath "phase41_test_output.txt"
    Write-Host "Tests complete. Output saved to phase41_test_output.txt"
    Get-Content -Path "phase41_test_output.txt"
} else {
    Write-Host "Build failed!"
    Get-Content -Path "phase41_build.txt" | Select-Object -Last 50
}
