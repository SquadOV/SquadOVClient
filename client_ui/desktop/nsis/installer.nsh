!macro customInstall
    File /oname=$PLUGINSDIR\VC_redist.x64.exe "${BUILD_RESOURCES_DIR}\VC_redist.x64.exe"
    ExecWait '"$PLUGINSDIR\VC_redist.x64.exe" /quiet /norestart'

    Var /GLOBAL sDumpFolder
    Var /GLOBAL cDumpFolder
    Var /GLOBAL sDumpCount
    Var /GLOBAL cDumpCount
    Var /GLOBAL sDumpType
    Var /GLOBAL cDumpType

    ReadRegStr $sDumpFolder HKLM "SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\SquadOV.exe" "DumpFolder"
    ReadRegStr $cDumpFolder HKLM "SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\squadov_client_service.exe" "DumpFolder"

    ReadRegDWORD $sDumpCount HKLM "SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\SquadOV.exe" "DumpCount"
    ReadRegDWORD $cDumpCount HKLM "SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\squadov_client_service.exe" "DumpCount"

    ReadRegDWORD $sDumpType HKLM "SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\SquadOV.exe" "DumpType"
    ReadRegDWORD $cDumpType HKLM "SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\squadov_client_service.exe" "DumpType"
    
    StrCmp $sDumpFolder "%APPDATA%\SquadOV\WERDumps" 0 registry
    StrCmp $cDumpFolder "%APPDATA%\SquadOV\WERDumps" 0 registry
    IntCmp $sDumpCount 10 0 registry registry
    IntCmp $cDumpCount 10 0 registry registry
    IntCmp $sDumpType 1 0 registry registry
    IntCmp $cDumpType 1 0 registry registry
    Goto finish
    registry:
        File /oname=$PLUGINSDIR\advanced_crash_dump_enabler.exe "${BUILD_RESOURCES_DIR}\advanced_crash_dump_enabler.exe"
        ExecShell "open" '"$PLUGINSDIR\advanced_crash_dump_enabler.exe"'
    finish:
        !echo "Finish"
!macroend