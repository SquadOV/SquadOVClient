export enum HearthstoneGameStep {
    Invalid,
    BeginFirst,
    BeginShuffle,
    BeginDraw,
    BeginMulligan,
    MainBegin,
    MainReady,
    MainResource,
    MainDraw,
    MainStart,
    MainAction,
    MainCombat,
    MainEnd,
    MainNext,
    FinalWrapup,
    FinalGameover,
    MainCleanup,
	MainStartTriggers
}

export function hearthstoneGameStepFromString(s: string): HearthstoneGameStep {
    if (s == 'BEGIN_FIRST') {
        return HearthstoneGameStep.BeginFirst
    } else if (s == 'BEGIN_SHUFFLE') {
        return HearthstoneGameStep.BeginShuffle
    } else if (s == 'BEGIN_DRAW') {
        return HearthstoneGameStep.BeginDraw
    } else if (s == 'BEGIN_MULLIGAN') {
        return HearthstoneGameStep.BeginMulligan
    } else if (s == 'MAIN_BEGIN') {
        return HearthstoneGameStep.MainBegin
    } else if (s == 'MAIN_READY') {
        return HearthstoneGameStep.MainReady
    } else if (s == 'MAIN_RESOURCE') {
        return HearthstoneGameStep.MainResource
    } else if (s == 'MAIN_DRAW') {
        return HearthstoneGameStep.MainDraw
    } else if (s == 'MAIN_START') {
        return HearthstoneGameStep.MainStart
    } else if (s == 'MAIN_ACTION') {
        return HearthstoneGameStep.MainAction
    } else if (s == 'MAIN_COMBAT') {
        return HearthstoneGameStep.MainCombat
    } else if (s == 'MAIN_END') {
        return HearthstoneGameStep.MainEnd
    } else if (s == 'MAIN_NEXT') {
        return HearthstoneGameStep.MainNext
    } else if (s == 'FINAL_WRAPUP') {
        return HearthstoneGameStep.FinalWrapup
    } else if (s == 'FINAL_GAMEOVER') {
        return HearthstoneGameStep.FinalGameover
    } else if (s == 'MAIN_CLEANUP') {
        return HearthstoneGameStep.MainCleanup
    } else if (s == 'MAIN_START_TRIGGERS') {
        return HearthstoneGameStep.MainStartTriggers
    } else {
        return HearthstoneGameStep.Invalid
    }
}

export function isGameStepMulligan(s : HearthstoneGameStep): boolean {
    return (s == HearthstoneGameStep.BeginMulligan)
}