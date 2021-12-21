export enum AnalyticsCategory {
    Button,
    MatchVod,
    MatchInfo,
    Share,
    SetupWizard,
}

export enum AnalyticsAction {
    // Buttons/Generic
    CreateSquad,
    InviteSquad,
    CopyReferral,
    ReadNewsUpdate,
    // Vod
    PlayVod,
    StopVod,
    DeleteVod,
    UploadVod,
    DownloadVod,
    StartDrawVod,
    EndDrawVod,
    OpenCreateClip,
    CreateClip,
    SaveClip,
    ChangePov,
    // Match Info
    GoToPhase,
    GoToEvent,
    GoToTimeline,
    NavigateMatchInfo,
    // Sharing
    ShareMatchClip,
    CreateSharePublic,
    DeleteSharePublic,
    CopySharePublic,
    ShareSocialMedia,
    // Setup Wizard
    StartSetupWizard,
    SetupStartSelectGames,
    SetupStartRiot,
    SetupStartCsgo,
    SetupStartWow,
    SetupStartSquads,
    SetupStartDownloadDiscord,
    SetupDoDownload,
    SetupJoinDiscord,
    FinishSetupWizard,
    // Other Vod
    VodTimeUpdate,
}