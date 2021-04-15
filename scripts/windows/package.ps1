Write-Host "Begin Packaging Windows SquadOV"

Set-Location -Path ..\..
cmake -S . -B build -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target squadov_client_service
cmake --build build --config Release --target advanced_crash_dump_enabler

Set-Location -Path .\build\bin\x64\Release
Start-Process -FilePath "signtool.exe" -ArgumentList "sign","/f","$env:CSC_LINK","/p","$env:CSC_KEY_PASSWORD","squadov_client_service.exe" -Wait -NoNewWindow
Start-Process -FilePath "signtool.exe" -ArgumentList "sign","/f","$env:CSC_LINK","/p","$env:CSC_KEY_PASSWORD","advanced_crash_dump_enabler.exe" -Wait -NoNewWindow
Start-Process -FilePath "signtool.exe" -ArgumentList "sign","/f","$env:CSC_LINK","/p","$env:CSC_KEY_PASSWORD","ffmpeg.exe" -Wait -NoNewWindow

Set-Location -Path ..\..\..\..\client_ui\common_web
yarn install --check-files
Remove-Item .\dist\production -Recurse
yarn run webpack --config ./webpack/squadov.config.js --env.target=electron-renderer

Set-Location -Path ..\desktop
yarn install --check-files
Remove-Item .\dist -Recurse
Remove-Item .\assets -Recurse
Copy-Item -Path "..\assets" -Destination "assets" -Recurse -Force
Copy-Item -Path "..\common_web\dist\production\electron-renderer" -Destination dist -Recurse -Force
Copy-Item -Path "../../build/bin/x64/Release/advanced_crash_dump_enabler.exe" -Destination nsis
yarn run dist --config ..\..\scripts\windows\electron-builder.yml --publish always

Set-Location -Path ..\..\scripts\windows
Write-Host "Finish Packaging Windows SquadOV"