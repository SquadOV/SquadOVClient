$GCP_PROJECT=$args[0]
Set-Item -Path Env:GCP_PROJECT -Value $GCP_PROJECT

$PUBLISH=$args[1]

Write-Host "Begin Packaging Windows SquadOV $GCP_PROJECT"

Set-Location -Path ..\..
cmake -S . -B build -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target squadov_client_service

Set-Location -Path .\client_ui\common_web
Remove-Item .\dist\production -Recurse
yarn run webpack --config ./webpack/$GCP_PROJECT.config.js --env.target=electron-renderer

Set-Location -Path ..\desktop
Remove-Item .\dist -Recurse
Remove-Item .\assets -Recurse
Copy-Item -Path "..\assets" -Destination "assets" -Recurse -Force
Copy-Item -Path "..\common_web\dist\production\electron-renderer" -Destination dist -Recurse -Force
yarn run dist --config ..\..\scripts\windows\electron-builder.yml --publish $PUBLISH

Set-Location -Path ..\..\scripts\windows
Write-Host "Finish Packinag Windows SquadOV"