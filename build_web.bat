@echo off
setlocal

echo === Black Hole Engine - WebGL build ===

REM 1) Pokud em++ v PATH
where em++ >nul 2>nul
if %errorlevel%==0 goto build

REM 2) Najít EMSDK variable
if not "%EMSDK%"=="" (
    call "%EMSDK%\emsdk_env.bat"
    goto build
)

REM 3) Fallback
if exist "%USERPROFILE\emsdk\emsdk_env.bat"
(
    call "%USERPROFILE\emsdk\emsdk_env.bat"
    goto build
)

echo ERROR: em++ not found. Run emsdk_env.bat first or set EMSDK env var.
exit /b 1

:build
echo Building WebGL version...

@echo off

em++ ^
 src/engine.cpp ^
 src/models/shader.cpp ^
 src/models/sphere.cpp ^
 src/models/stb_image.cpp ^
 src/models/texture.cpp ^
 web/main_web.cpp ^
 -Iinclude -Isrc ^
 -s USE_GLFW=3 ^
 -s FULL_ES3=1 ^
 -s ALLOW_MEMORY_GROWTH=1 ^
 -s ASSERTIONS=1 ^
 --preload-file src/shaders_web@/shaders ^
 --preload-file src/assets@/assets ^
 -o web/index.html

echo Build finished.
echo Run local server:
echo    cd web
echo    python -m http.server 8000
endlocal