:PREP

	for /f "tokens=1,2 delims=_" %%a in ("%JOB_NAME%") do set tsk=%%a&set vers=%%b

	set TASK=%tsk%
	set VERSION=%vers%

	GOTO VS


:VS

	IF "%VERSION%" == "Trunk" (
		GOTO USE_VS17
	)
	IF EXIST "c:\Program Files (x86)\Microsoft Visual Studio 8" GOTO USE_VS8
	IF EXIST "c:\Program Files (x86)\Microsoft Visual Studio 9.0" GOTO USE_VS9
	@echo "unsure of VS version"
	GOTO Error

:USE_VS8
	@echo Using VS8
	set VSDIR=c:\Program Files (x86)\Microsoft Visual Studio 8
	GOTO BUILD

:USE_VS9
	@echo Using VS9
	set VSDIR=c:\Program Files (x86)\Microsoft Visual Studio 9.0
	GOTO BUILD

:USE_VS14
	@echo Using VS14
	set VSDIR=c:\Program Files (x86)\Microsoft Visual Studio 14.0
	GOTO BUILD

:USE_VS17
	@echo Using VS17
	set VSDIR=C:\Program Files\Microsoft Visual Studio\2022\Professional
	GOTO BUILD

:BUILD

	set WORKSPACE=%CD%

	for /f "skip=1" %%d in ('wmic os get localdatetime') do if not defined mydate set mydate=%%d
	set BDATE=%mydate:~0,8%-%mydate:~8,4%

	set VS80COMNTOOLS=%VSDIR%\Common7\Tools\

	@echo %PATH%
	IF EXIST "%VSDIR%\VC\vcvarsall.bat" call "%VSDIR%\VC\vcvarsall.bat"
	IF EXIST "%VSDIR%\VC\Auxiliary\Build\vcvarsall.bat" call "%VSDIR%\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64
	if %ERRORLEVEL% NEQ 0 GOTO Error

	for  %%x in (Release,Debug) do (
		echo %%x
		for  %%y in (x86,x64) do (
			echo %%x %%y
			msbuild %WORKSPACE%\kafka.sln /p:Configuration=%%x;Platform=%%y
		)
	)



	if %ERRORLEVEL% NEQ 0 GOTO Error

	set /a RESULT=0
	GOTO End

:Error
	set /a RESULT=1

:End
	EXIT /B %RESULT%
