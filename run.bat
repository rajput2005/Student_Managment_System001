@echo off
echo ========================================================
echo   Launching EduPulse Student Management System
echo ========================================================

if not exist server.exe (
    echo [INFO] server.exe not found. Running build.bat first...
    call build.bat
    if not exist server.exe exit /b 1
)

echo Starting C++ Backend Server on http://localhost:8080 ...
start "" http://localhost:8080
server.exe
