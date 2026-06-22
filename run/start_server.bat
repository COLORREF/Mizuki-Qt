@echo off
cd /d "%~dp0server"
echo ============================================================
echo   Django backend starting...
echo   Open: http://127.0.0.1:8000/
echo   Press Ctrl+C to stop
echo ============================================================
echo.
"..\python\python.exe" manage.py runserver 0.0.0.0:8000