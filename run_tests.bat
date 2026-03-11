@echo off
setlocal enabledelayedexpansion

echo Running Effects Phase 15 Test >> test_results.txt
cd /d c:\DJROOFRAT\DJ-ROOFRAT
build-vs\Debug\Effects_Phase15_test.exe >> test_results.txt 2>&1

echo. >> test_results.txt
echo Running Effects Phase 16 Test >> test_results.txt
build-vs\Debug\Effects_Phase16_test.exe >> test_results.txt 2>&1

echo. >> test_results.txt
echo All tests completed >> test_results.txt
type test_results.txt
