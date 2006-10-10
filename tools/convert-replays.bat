@echo off

setlocal enableextensions

    set DCEXEC=democonv

    if (%1) == () (
    :usage
        echo Usage: %0 [options] [file-list]
        %DCEXEC% --help 2>&1 | more +1
        echo   --prefix ^<pref^>
        echo            Add this prefix before filenames of converted
        echo            replays.  Useful for saving replays to a different
        echo            directory instead of CWD.
        exit /b 1
    )

    rem do .. while ..
    :args
        if (%1) == (-h) (
            goto usage
        ) else if (%1) == (--help) (
            goto usage
        ) else if (%1) == (--goal) (
            set DCEXEC=%DCEXEC% %1
        ) else if (%1) == (--fall-out) (
            set DCEXEC=%DCEXEC% %1
        ) else if (%1) == (--time-out) (
            set DCEXEC=%DCEXEC% %1
        ) else if (%1) == (--best-time) (
            set DCEXEC=%DCEXEC% %1
        ) else if (%1) == (--most-coins) (
            set DCEXEC=%DCEXEC% %1
        ) else if (%1) == (--freestyle) (
            set DCEXEC=%DCEXEC% %1
        ) else if (%1) == (--player) (
            set DCEXEC=%DCEXEC% %1 %2
            shift
        ) else if (%1) == (--date) (
            set DCEXEC=%DCEXEC% %1 %2
            shift
        ) else if (%1) == (--prefix) (
            set PREFIX=%2
            shift
        )
        shift
    if not (%1) == () goto args

    for %%d in (%*) do (
        rem This ugly test tries to make sure we're not processing command
        rem line options and directories (weird errors pop up).
        if exist %%d if not exist %%dnul (
            %DCEXEC% < %%d > %PREFIX%%%~nxd.nbr
            if errorlevel 1 del %PREFIX%%%~nxd.nbr
        )
    )
    exit /b 0

endlocal

rem vim:set sts=4 sw=4 et:
