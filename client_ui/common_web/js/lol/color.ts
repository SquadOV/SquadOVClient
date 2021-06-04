import { Color } from '@client/js/color'
import { LolMatch, getTeamIdFromParticipantId } from '@client/js/lol/matches'
import { LolMatchEvent } from '@client/js/lol/timeline'

export function getLolBlueTeamColor(): Color {
    return {
        r: 0,
        g: 0,
        b: 255,
    }
}

export function getLolRedTeamColor(): Color {
    return {
        r: 255,
        g: 0,
        b: 0,
    }
}

export function computeColorForLolEvent(match: LolMatch, e: LolMatchEvent, currentParticipantId: number | null | undefined) : string {
    let benefitsMyTeam = false
    let isMe = false

    // Style it so that green means the event is beneficial for my team and red 
    // means that it's deterimental. Yellow means it's me.
    if (!!currentParticipantId) {
        let myTeam = getTeamIdFromParticipantId(match, currentParticipantId)
        if (!!myTeam) {
            if (!!e.killerId) {
                benefitsMyTeam = (myTeam === getTeamIdFromParticipantId(match, e.killerId))
            } else if (!!e.teamId) {
                if (e.type == 'BUILDING_KILL') {
                    benefitsMyTeam = (myTeam !== e.teamId)
                } else {
                    benefitsMyTeam = (myTeam === e.teamId)
                }
            }        
        }
        isMe = e.killerId === currentParticipantId || 
            e.victimId === currentParticipantId || 
            (!!e.assistingParticipantIds && e.assistingParticipantIds.includes(currentParticipantId))
    } else {
        if (!!e.killerId) {
            benefitsMyTeam = (100 === getTeamIdFromParticipantId(match, e.killerId))
        } else if (!!e.teamId) {
            if (e.type == 'BUILDING_KILL') {
                benefitsMyTeam = (100 !== e.teamId)
            } else {
                benefitsMyTeam = (100 === e.teamId)
            }
        }
    }

    
    if (!!currentParticipantId) {
        return isMe ? 'color-first-place' :
                benefitsMyTeam ? 'color-top-place' : 
                    'color-bottom-place'
    } else {
        return benefitsMyTeam ? 'color-blue-team' : 'color-red-team'
    }
}