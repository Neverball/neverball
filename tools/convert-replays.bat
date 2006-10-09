@echo off

setlocal enableextensions
setlocal enabledelayedexpansion

    rem XXX Usage messages really should be democonv's business.
    if (%1) == () goto usage

    set DCEXEC=democonv

    rem do .. while ..
    :args
        if (%1) == (--goal) (
            set DCEXEC=!DCEXEC! --goal
        ) else if (%1) == (--fall-out) (
            set DCEXEC=!DCEXEC! --fall-out
        ) else if (%1) == (--time-out) (
            set DCEXEC=!DCEXEC! --time-out
        ) else if (%1) == (--best-time) (
            set DCEXEC=!DCEXEC! --best-time
        ) else if (%1) == (--most-coins) (
            set DCEXEC=!DCEXEC! --most-coins
        ) else if (%1) == (--freestyle) (
            set DCEXEC=!DCEXEC! --freestyle
        ) else if (%1) == (--player) (
            set DCEXEC=!DCEXEC! --player %2
            shift
        ) else if (%1) == (--date) (
            set DCEXEC=!DCEXEC! --date %2
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

    :usage

        echo %0 [options] [file-list]
        echo Options:
        echo                Invoked with no options, outputs this usage message.
        echo   --goal
        echo   --fall-out
        echo   --time-out
        echo                Outcome of the replay.
        echo   --best-time
        echo   --most-coins
        echo   --freestyle
        echo                Type of the replay.
        echo   --player name
        echo                Name of the player.  Max 8 characters.
        echo   --date yyyy-mm-ddTHH:MM:SS
        echo                Date of creation. "T" is literally the character T.
        echo                All fields are required, if this option is used.

        exit /b 1
                
endlocal
endlocal

rem vim:set sts=4 sw=4 et:
