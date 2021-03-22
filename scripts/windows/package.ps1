$GCP_PROJECT=$args[0]
Set-Item -Path Env:GCP_PROJECT -Value $GCP_PROJECT

$PUBLISH=$args[1]

Write-Host "Begin Packaging Windows SquadOV $GCP_PROJECT"

Set-Location -Path ..\..
cmake -S . -B build -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target squadov_client_service
cmake --build build --config Release --target advanced_crash_dump_enabler

Set-Location -Path .\client_ui\common_web
yarn install --check-files
Remove-Item .\dist\production -Recurse
yarn run webpack --config ./webpack/$GCP_PROJECT.config.js --env.target=electron-renderer

Set-Location -Path ..\desktop
yarn install --check-files
Remove-Item .\dist -Recurse
Remove-Item .\assets -Recurse
Copy-Item -Path "..\assets" -Destination "assets" -Recurse -Force
Copy-Item -Path "..\common_web\dist\production\electron-renderer" -Destination dist -Recurse -Force
Copy-Item -Path "../../build/bin/Release/advanced_crash_dump_enabler.exe" -Destination nsis
yarn run dist --config ..\..\scripts\windows\electron-builder.yml --publish $PUBLISH

Set-Location -Path ..\..\scripts\windows
Write-Host "Finish Packaging Windows SquadOV"