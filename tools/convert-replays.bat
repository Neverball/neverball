@echo off

setlocal enableextensions

    set DCEXEC=democonv

    if (%1) == () (
    :usage
        echo Usage: %0 [options] [file-list] >&2
        %DCEXEC% --help 2>&1 | more +1 >&2
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
        )
        shift
    if not (%1) == () goto args

    for %%d in (%*) do (
        if exist %%d (
            %DCEXEC% < %%d > %%d.nbr
            if errorlevel 1 del %%d.nbr
        )
    )
    exit /b 0

endlocal

rem vim:set sts=4 sw=4 et:
