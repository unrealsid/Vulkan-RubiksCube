@echo off
setlocal enabledelayedexpansion

:: Validate input
if "%~2"=="" (
    echo Usage: %~nx0 path_to_shader_folder path_to_glslc_exe
    exit /b 1
)

set "SHADER_DIR=%~1"
set "GLSLC=%~2"

if not exist "%GLSLC%" (
    echo Error: glslc not found at "%GLSLC%"
    exit /b 1
)

echo Compiling .glsl files in "%SHADER_DIR%" using glslc at "%GLSLC%"...

:: Loop over all .glsl files recursively
for /R "%SHADER_DIR%" %%F in (*.glsl) do (
    set "FILE=%%F"
    set "EXT=%%~xF"
    set "NAME=%%~nxF"
    set "OUT=%%~dpnF.spv"

    :: Determine shader stage from filename
    set "STAGE="

    echo !NAME! | findstr /i ".vert.glsl" >nul && set "STAGE=vertex"
    echo !NAME! | findstr /i ".frag.glsl" >nul && set "STAGE=fragment"
    echo !NAME! | findstr /i ".geom.glsl" >nul && set "STAGE=geometry"
    echo !NAME! | findstr /i ".comp.glsl" >nul && set "STAGE=compute"
    echo !NAME! | findstr /i ".tesc.glsl" >nul && set "STAGE=tesscontrol"
    echo !NAME! | findstr /i ".tese.glsl" >nul && set "STAGE=tesseval"

    if defined STAGE (
        echo Compiling !FILE! as !STAGE! to !OUT!...
        "%GLSLC%" -fshader-stage=!STAGE! "!FILE!" -o "!OUT!"
        if errorlevel 1 (
            echo [ERROR] Failed to compile !FILE!
        )
    ) else (
        echo [SKIP] Could not determine stage for !FILE! — rename like *.vert.glsl or *.frag.glsl
    )
)

echo Done.