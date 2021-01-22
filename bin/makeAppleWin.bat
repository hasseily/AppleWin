@IF "%~1" == "" GOTO help
@IF "%APPLEWIN_ROOT%" == "" GOTO help2

@MKDIR "%~1"
@COPY /Y "%APPLEWIN_ROOT%\bin\A2_BASIC.SYM" "%~1"
@COPY /Y "%APPLEWIN_ROOT%\bin\APPLE2E.SYM" "%~1"
@COPY /Y "%APPLEWIN_ROOT%\bin\DELREG.INF" "%~1"
@COPY /Y "%APPLEWIN_ROOT%\bin\DebuggerAutoRun.txt" "%~1"
@COPY /Y "%APPLEWIN_ROOT%\bin\GNU General Public License.txt" "%~1"
@COPY /Y "%APPLEWIN_ROOT%\bin\History.txt" "%~1"
@COPY /Y "%APPLEWIN_ROOT%\bin\MASTER.DSK" "%~1"
@COPY /Y "%APPLEWIN_ROOT%\docs\Debugger_Changelog.txt" "%~1"
@COPY /Y "%APPLEWIN_ROOT%\help\AppleWin.chm" "%~1"
@COPY /Y "%APPLEWIN_ROOT%\Release\AppleWin.exe" "%~1"
@COPY /Y "%APPLEWIN_ROOT%\Release\HookFilter.dll" "%~1"
CD "%~1"
"C:\Program Files\7-Zip\7z.exe" a ..\AppleWin"%~1".zip *
"C:\Program Files\7-Zip\7z.exe" a ..\AppleWin"%~1"-PDB.zip "%APPLEWIN_ROOT%\Release\AppleWin.pdb" "%APPLEWIN_ROOT%\Release\HookFilter.pdb"
CD ..
@GOTO end

:help
@ECHO %0 "<new version>"
@ECHO EG: %0 1.29.8.0
@GOTO end

:help2
@ECHO APPLEWIN_ROOT env variable is not defined

:end
