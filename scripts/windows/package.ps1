Write-Host "Begin Packaging Windows SquadOV"

Set-Location -Path ..\..
cmake -S . -B build -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target squadov_client_service

Set-Location -Path .\client_ui
npm run prodbuild
npm run dist -- --config ..\scripts\windows\electron-builder.yml --publish always

Set-Location -Path ..\scripts\windows
Write-Host "Finish Packinag Windows SquadOV"