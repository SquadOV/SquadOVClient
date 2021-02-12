For ($i=0; $i -le 100; $i++) {
    Write-Host "Run VOD Tester $i"
    Start-Process -FilePath  "A:\Git\SquadOVClient\build\bin\Debug\vod_record_tester.exe" -ArgumentList "--process","obs64.exe","--mode","NORMAL","--duration","30","--output","C:\Users\Michael Bao\Videos\normal.ts" -Wait
}