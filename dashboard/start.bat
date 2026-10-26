@echo off
setlocal
cd /d "%~dp0"

echo ===================================================
echo   C-- Compiler Logic Verifier Local Dashboard
echo ===================================================
echo.

if not exist "node_modules" (
    echo [INFO] Installing dashboard dependencies...
    call npm install
    if errorlevel 1 (
        echo [ERROR] Failed to install dependencies.
        pause
        exit /b 1
    )
)

if not exist "dist\index.html" (
    echo [INFO] Building dashboard frontend...
    call npm run build
    if errorlevel 1 (
        echo [ERROR] Failed to build frontend.
        pause
        exit /b 1
    )
)

echo.
echo [INFO] Launching dashboard server at http://127.0.0.1:4000 ...
echo [INFO] Opening default browser...
start "" "http://127.0.0.1:4000"

node server/server.js

