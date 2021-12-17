Write-Host "Begin Packaging Windows SquadOV"

Set-Location -Path ..\..
cmake -S . -B build-release -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=A:\Git\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build build-release --config Release --target squadov_client_service
cmake --build build-release --config Release --target advanced_crash_dump_enabler
cmake --build build-release --config Release --target csgo_demo_retriever
cmake --build build-release --config Release --target crashpad_handler
cmake --build build-release --config Release --target speed_check
cmake --build build-release --config Release --target audio_sanity_checker

Set-Location -Path .\build-release\bin\x64\Release
Set-Location -Path ..\..\..\..\client_ui\common_web
yarn install --check-files
Remove-Item .\dist\production -Recurse
yarn run webpack --config ./webpack/production.config.js --env target=electron-renderer

Set-Location -Path ..\desktop
yarn install --check-files
Remove-Item .\dist -Recurse
Remove-Item .\assets -Recurse
Copy-Item -Path "..\assets" -Destination "assets" -Recurse -Force
Copy-Item -Path "..\common_web\dist\production\electron-renderer" -Destination dist -Recurse -Force
Copy-Item -Path "../../build-release/bin/x64/Release/advanced_crash_dump_enabler.exe" -Destination nsis
yarn run dist --config ..\..\scripts\windows\electron-builder.yml --publish always

Set-Location -Path ..\..\scripts\windows
Write-Host "Finish Packaging Windows SquadOV"