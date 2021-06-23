export enum AnalyticsCategory {
    Button,
    MatchVod,
    MatchInfo,
    Share,
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
    CreateClip,
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
}