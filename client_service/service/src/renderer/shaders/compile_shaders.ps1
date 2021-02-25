$files = Get-ChildItem -Path ".\*.hlsl" ;

foreach ($file in $files) {
    $item = Get-Item $file;
    $basename = $item.BaseName;

    if ($basename -like "*.vs") {
        Write-Output "Compiling VS $basename.hlsl -> $basename.fxc"
        Start-Process -NoNewWindow -FilePath "C:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x86\fxc.exe" -ArgumentList "/T", "vs_5_0", "/Fo", "$basename.fxc", "/O3", "$basename.hlsl" -Wait;
    } else {
        Write-Output "Compiling PS $basename.hlsl -> $basename.fxc"
        Start-Process -NoNewWindow -FilePath "C:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x86\fxc.exe" -ArgumentList "/T", "ps_5_0", "/Fo", "$basename.fxc", "/O3", "$basename.hlsl" -Wait;
    }
}