export interface Squad {
    id: number
    squadName: string
    squadGroup: string
    creationTime: Date
}

export interface SquadInvite {
    squadId: number
    userId: number
    joined: boolean
    responseTime: Date | null
    inviteTime: Date | null
    inviteUuid: string
}