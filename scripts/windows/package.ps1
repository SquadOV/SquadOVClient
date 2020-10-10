$GCP_PROJECT=$args[0]
$PUBLISH=$args[1]

Write-Host "Begin Packaging Windows SquadOV $GCP_PROJECT"

Set-Location -Path ..\..
cmake -S . -B build -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target squadov_client_service

Set-Location -Path .\client_ui
yarn run webpack --config ./webpack/$GCP_PROJECT.config.js
yarn run dist -- --config ..\scripts\windows\electron-builder.yml --publish $PUBLISH

Set-Location -Path ..\scripts\windows
Write-Host "Finish Packinag Windows SquadOV"