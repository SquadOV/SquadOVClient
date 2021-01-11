import { apiClient, ApiData } from '@client/js/api'
import { getSquadOVUser, SquadOVUser } from '@client/js/squadov/user'
import * as ls from 'local-storage'

export function checkHasSessionCookie() : boolean {
    let cookie = ls.get<any>('squadov-session')
    return !!cookie && !!cookie.sessionId
}

// In the web case, we need to check to see if the session is set in a cookie instead
// of loading it locally. If it's not there (or we can't grab the user info),
// we assume the user is logged out and direct them to the login page. The onValid and
// onInvalid callback functions allow us to control what to do when we encounter a
// valid or invalid session on a per-page basis.
export function loadInitialSessionFromCookies(store: any, onValid: () => void, onInvalid: () => void) {
    let cookie = ls.get<any>('squadov-session')
    if (!cookie) {
        onInvalid()
        return
    }

    apiClient.setSessionFull(cookie.sessionId, parseInt(cookie.userId))
    getSquadOVUser(parseInt(cookie.userId)).then((resp : ApiData<SquadOVUser>) => {
        store.commit('setUser' , resp.data)
        onValid()
    }).catch((err : any ) => {
        console.log('Failed to obtain user: ', err)
        onInvalid()
    })
}

export function storeSessionCookie(sessionId: string, userId: number) {
    ls.set('squadov-session', {
        sessionId,
        userId
    })
}

export function getSessionId(): string {
    let cookie = ls.get<any>('squadov-session')
    if (!cookie) {
        return ''
    }
    return cookie.sessionId
}