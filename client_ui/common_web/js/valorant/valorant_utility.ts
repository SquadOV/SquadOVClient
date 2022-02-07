export function formatRoundTime(ms : number) : string {
    const totalSeconds = Math.floor(ms / 1000.0)
    const minutes = `${Math.floor(totalSeconds / 60.0)}`
    const seconds = `${totalSeconds % 60}`.padStart(2, '0')
    return `${minutes}:${seconds}`
}

export const VALID_VALORANT_RANKS = [
    {tier: 0, name: 'Unranked'},
    {tier: 3, name: 'Iron 1'},
    {tier: 4, name: 'Iron 2'},
    {tier: 5, name: 'Iron 3'},
    {tier: 6, name: 'Bronze 1'},
    {tier: 7, name: 'Bronze 2'},
    {tier: 8, name: 'Bronze 3'},
    {tier: 9, name: 'Silver 1'},
    {tier: 10, name: 'Silver 2'},
    {tier: 11, name: 'Silver 3'},
    {tier: 12, name: 'Gold 1'},
    {tier: 13, name: 'Gold 2'},
    {tier: 14, name: 'Gold 3'},
    {tier: 15, name: 'Platnium 1'},
    {tier: 16, name: 'Platnium 2'},
    {tier: 17, name: 'Platnium 3'},
    {tier: 18, name: 'Diamond 1'},
    {tier: 19, name: 'Diamond 2'},
    {tier: 20, name: 'Diamond 3'},
    {tier: 21, name: 'Immortal 1'},
    {tier: 22, name: 'Immortal 2'},
    {tier: 23, name: 'Immortal 3'},
    {tier: 24, name: 'Radiant'},
]

const rankTierToName = new Map(VALID_VALORANT_RANKS.map((ele) => [ele.tier, ele.name]))

export function valorantRankToName(tier: number): string {
    let nm = rankTierToName.get(tier)
    if (!nm) {
        return 'Unknown'
    }
    return nm
}

export enum ValorantAgentRole {
    Controller,
    Duelist,
    Initiator,
    Sentinel,
    Unknown,
}

export const ALL_VALORANT_AGENT_ROLES = [
    ValorantAgentRole.Controller,
    ValorantAgentRole.Duelist,
    ValorantAgentRole.Initiator,
    ValorantAgentRole.Sentinel,
    ValorantAgentRole.Unknown
]

export function valorantRoleToName(role: ValorantAgentRole) {
    switch (role) {
        case ValorantAgentRole.Controller:
            return 'Controller'
        case ValorantAgentRole.Duelist:
            return 'Duelist'
        case ValorantAgentRole.Initiator:
            return 'Initiator'
        case ValorantAgentRole.Sentinel:
            return 'Sentinel'
        case ValorantAgentRole.Unknown:
            return 'Any'
    }
}

export const AGENT_ROLE_MAPPING = [
    {id: '5F8D3A7F-467B-97F3-062C-13ACF203C006', role: ValorantAgentRole.Initiator},
    {id: 'F94C3B30-42BE-E959-889C-5AA313DBA261', role: ValorantAgentRole.Duelist},
    {id: '22697A3D-45BF-8DD7-4FEC-84A9E28C69D7', role: ValorantAgentRole.Sentinel},
    {id: '601DBBE7-43CE-BE57-2A40-4ABD24953621', role: ValorantAgentRole.Duelist},
    {id: '6F2A04CA-43E0-BE17-7F36-B3908627744D', role: ValorantAgentRole.Initiator},
    {id: '117ED9E3-49F3-6512-3CCF-0CADA7E3823B', role: ValorantAgentRole.Sentinel},
    {id: '320B2A48-4D9B-A075-30F1-1F93A9B638FA', role: ValorantAgentRole.Initiator},
    {id: '1E58DE9C-4950-5125-93E9-A0AEE9F98746', role: ValorantAgentRole.Sentinel},
    {id: '707EAB51-4836-F488-046A-CDA6BF494859', role: ValorantAgentRole.Controller},
    {id: 'EB93336A-449B-9C1B-0A54-A891F7921D69', role: ValorantAgentRole.Duelist},
    {id: '41FB69C1-4189-7B37-F117-BCAF1E96F1BF', role: ValorantAgentRole.Controller},
    {id: '9F0D8BA9-4140-B941-57D3-A7AD57C6B417', role: ValorantAgentRole.Controller},
    {id: '7F94D92C-4234-0A36-9646-3A87EB8B5C89', role: ValorantAgentRole.Duelist},
    {id: '569FDD95-4D10-43AB-CA70-79BECC718B46', role: ValorantAgentRole.Sentinel},
    {id: 'A3BFB853-43B2-7238-A4F1-AD90E9E46BCC', role: ValorantAgentRole.Duelist},
    {id: '8E253930-4C05-31DD-1B6C-968525494517', role: ValorantAgentRole.Controller},
    {id: 'ADD6443A-41BD-E414-F6AD-E58D267F4E95', role: ValorantAgentRole.Duelist},
    {id: 'BB2A4828-46EB-8CD1-E765-15848195D751', role: ValorantAgentRole.Duelist},
]

const agentToRole = new Map(AGENT_ROLE_MAPPING.map((ele) => [ele.id, ele.role]))
const roleToAgents = new Map(ALL_VALORANT_AGENT_ROLES.map((role) => [role, AGENT_ROLE_MAPPING.filter((ele) => ele.role === role).map((ele) => ele.id)]))

export function getProbableRoleFromMultipleAgents(agents: string[]): ValorantAgentRole {
    let votes: Map<ValorantAgentRole, number> = new Map()
    for (let a of agents) {
        let role = agentToRole.get(a)
        if (role === undefined) {
            role = ValorantAgentRole.Unknown
        }

        if (!votes.has(role)) {
            votes.set(role, 0)
        }

        votes.set(role, votes.get(role)! + 1)
    }

    let bestRole = ValorantAgentRole.Unknown
    let mostVotes = 0
    for (let [role, count] of votes) {
        if (count > mostVotes) {
            mostVotes = count
            bestRole = role
        }
    }
    return bestRole
}

export function getAgentsFromRole(role: ValorantAgentRole): string[] {
    return roleToAgents.get(role)!
}

export function getAvailableAgents(): string[] {
    return AGENT_ROLE_MAPPING.map((ele) => ele.id)
}