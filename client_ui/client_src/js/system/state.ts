export interface SquadOvState {
    paused: boolean
    recordingGames: string[]
    runningGames: string[]
}

export function createDefaultState(): SquadOvState {
    return {
        paused: false,
        recordingGames: [],
        runningGames: []
    }
}