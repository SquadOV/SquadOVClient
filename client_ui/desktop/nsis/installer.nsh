!macro customHeader
    RequestExecutionLevel admin
!macroend

!macro customInstall
    # Check if mini dump registry keys are installed for SquadOV.exe and squadov_client_service.exe.
    WriteRegExpandStr HKLM "SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\SquadOV.exe" "DumpFolder" "%APPDATA%\SquadOV\WERDumps"
    WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\SquadOV.exe" "DumpCount" 10
    WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\SquadOV.exe" "DumpType" 1

    WriteRegExpandStr HKLM "SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\squadov_client_service.exe" "DumpFolder" "%APPDATA%\SquadOV\WERDumps"
    WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\squadov_client_service.exe" "DumpCount" 10
    WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\squadov_client_service.exe" "DumpType" 1
!macroend