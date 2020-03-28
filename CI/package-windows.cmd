mkdir package
cd package

git log --pretty=format:'%h' -n 1 > package-version.txt
set /p PackageVersion=<package-version.txt

REM Package ZIP archive
7z a "obs-websocket-%PackageVersion%-Windows.zip" "..\release\*"

REM Build installer
iscc ..\installer\installer.iss /O. /F"obs-websocket-%PackageVersion%-Windows"

