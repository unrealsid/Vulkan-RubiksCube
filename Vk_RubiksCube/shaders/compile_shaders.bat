REM How to use this file:
REM ./compile_shaders.bat D:\"Visual Studio 2022"\Code\Vk_RubiksCube2\Vk_RubiksCube\shaders C:\VulkanSDK\1.4.321.1\Bin\glslc.exe D:\"Visual Studio 2022"\Code\Vk_RubiksCube2\Vk_RubiksCube\shaders\include"

@echo off
setlocal enabledelayedexpansion

:: Validate input
if "%~3"=="" (
    echo Usage: %~nx0 path_to_shader_folder path_to_glslc_exe path_to_include_dir
    exit /b 1
)

set "SHADER_DIR=%~1"
set "GLSLC=%~2"
set "INCLUDE_DIR=%~3"

if not exist "%GLSLC%" (
    echo Error: glslc not found at "%GLSLC%"
    exit /b 1
)

if not exist "%INCLUDE_DIR%" (
    echo Error: include directory not found at "%INCLUDE_DIR%"
    exit /b 1
)

echo Compiling shaders in "%SHADER_DIR%" using glslc at "%GLSLC%"...
echo Including headers from "%INCLUDE_DIR%"...

:: Loop over all .glsl files recursively
for /R "%SHADER_DIR%" %%F in (*.glsl) do (
    set "FILE=%%F"
    set "NAME=%%~nxF"
    set "OUT=%%~dpnF.spv"

    :: Determine shader stage
    set "STAGE="
    echo !NAME! | findstr /i ".vert.glsl" >nul && set "STAGE=vertex"
    echo !NAME! | findstr /i ".frag.glsl" >nul && set "STAGE=fragment"
    echo !NAME! | findstr /i ".geom.glsl" >nul && set "STAGE=geometry"
    echo !NAME! | findstr /i ".comp.glsl" >nul && set "STAGE=compute"
    echo !NAME! | findstr /i ".tesc.glsl" >nul && set "STAGE=tesscontrol"
    echo !NAME! | findstr /i ".tese.glsl" >nul && set "STAGE=tesseval"

    :: Compile if stage detected
    if defined STAGE (
        echo Compiling !FILE! as !STAGE! to !OUT!...
        "%GLSLC%" -fshader-stage=!STAGE! "!FILE!" -o "!OUT!" -I "%INCLUDE_DIR%"
        if errorlevel 1 (
            echo [ERROR] Failed to compile !FILE!
        )
    )
)

echo Done.