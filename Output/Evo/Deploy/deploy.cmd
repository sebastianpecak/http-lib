@echo off

rem ///////////////////////////////////////////////
rem Ensure parameters are defined
rem ///////////////////////////////////////////////
if "%1" == "" (
	echo Configuration is not defined!
	goto finish
)
if "%2" == "" (
	echo Output path is not defined!
	goto finish
)

rem ///////////////////////////////////////////////
rem Configuration
rem ///////////////////////////////////////////////
set OUTPATH=%2
if /I "%1" == "r" (
	SET DEBUG=0
) else (
	SET DEBUG=1
)

rem ///////////////////////////////////////////////
rem Print out configuration
echo ----------------------------
echo HTTPLIB deployment started.
echo ----------------------------
if %DEBUG% == 0 (
	echo 	Config: Release
) else (
	echo 	Config: Debug
)
echo Output: %OUTPATH%
echo ----------------------------

rem ///////////////////////////////////////////////
rem Create output folders structure.
rem ///////////////////////////////////////////////
if not exist "%OUTPATH%\F15" (
	mkdir "%OUTPATH%\F15"
)
if not exist "%OUTPATH%\I15" (
	mkdir "%OUTPATH%\I15"
)

rem ///////////////////////////////////////////////
rem Copy configuration-dependent files to output structure.
rem ///////////////////////////////////////////////
if %DEBUG% == 0 (
	copy ..\Files\Release\http.lib 					"%OUTPATH%\F15\http.lib" /Y
	copy ..\Files\Release\Sign\Flash\http.lib.p7s 	"%OUTPATH%\I15\http.lib.p7s" /Y
) else (
	copy ..\Files\Debug\http.lib 					"%OUTPATH%\F15\http.lib" /Y
	copy ..\Files\Debug\Sign\Flash\http.lib.p7s 	"%OUTPATH%\I15\http.lib.p7s" /Y
)

rem ///////////////////////////////////////////////
:finish

echo ----------------------------
echo HTTPLIB deployment finished.
echo ----------------------------
