@echo off
echo ========================================================
echo   Building EduPulse Student Management System (C++)
echo ========================================================

REM Check if sqlite3.o exists, if not compile wrapper
if not exist sqlite3.o (
    echo [1/2] Compiling SQLite amalgamation...
    gcc -O2 -c sqlite3_wrapper.c -o sqlite3.o
    if %ERRORLEVEL% neq 0 (
        echo [ERROR] Failed to compile SQLite amalgamation.
        pause
        exit /b 1
    )
) else (
    echo [1/2] SQLite object file already compiled (sqlite3.o).
)

echo [2/2] Compiling C++ backend server...
g++ -std=c++14 -O2 backend/main.cpp backend/database.cpp backend/http_server.cpp sqlite3.o -o server.exe -lws2_32
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Compilation failed. Please check error output.
    pause
    exit /b 1
)

echo.
echo ========================================================
echo   BUILD SUCCESSFUL! Created: server.exe
echo   Run 'run.bat' or 'server.exe' to launch the server!
echo ========================================================
pause
