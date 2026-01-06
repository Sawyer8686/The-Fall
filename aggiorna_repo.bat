@echo off
title Aggiornamento Repository Git - Unreal Engine 5
echo.
echo =========================================
echo  AGGIORNAMENTO REPOSITORY GIT (UE5)
echo =========================================
echo.

REM Posizionamento nella cartella del progetto
cd /d C:\UnrealProjects\TF

REM Controllo che Git sia disponibile
git --version >nul 2>&1
IF ERRORLEVEL 1 (
    echo ERRORE: Git non trovato nel PATH.
    pause
    exit /b
)

REM Mostra stato repository
git status
echo.

REM Aggiunta file (include Git LFS)
git add .

REM Richiesta messaggio di commit
set /p COMMIT_MSG=Inserisci il messaggio di commit: 

IF "%COMMIT_MSG%"=="" (
    echo ERRORE: Messaggio di commit vuoto.
    pause
    exit /b
)

REM Commit
git commit -m "%COMMIT_MSG%"

REM Push su GitHub
git push origin main

echo.
echo =========================================
echo  OPERAZIONE COMPLETATA
echo =========================================
pause

