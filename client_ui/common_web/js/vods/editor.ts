import { VodAssociation, VodMetadata } from '@client/js/squadov/vod'
import { v4 as uuidv4 } from 'uuid'
import { SquadOvGames } from '@client/js/squadov/game'

/// #if DESKTOP
import { ipcRenderer } from 'electron'
/// #endif

export function openVodEditingWindow(videoUuid: string, game: SquadOvGames) {
/// #if DESKTOP
    ipcRenderer.send('open-vod-editor', {
        videoUuid,
        game
    })
/// #else
    console.log('VOD Editing not yet supported on the web.')
/// #endif
}

// Returns a promise that returns the accessible file containing 
export function requestVodClip(source: string, start: number, end: number): Promise<{path: string, metadata: VodMetadata}> {
    let taskId = uuidv4()
    return new Promise((resolve, reject) => {
/// #if DESKTOP
        ipcRenderer.invoke('request-vod-clip', {
            task: taskId,
            source,
            start,
            end
        }).then(resolve).catch(reject)
/// #else
        reject('VOD Editing not yet supported on the web.')
/// #endif
    })

}

enum VodEditorMessageType {
    RequestVodAssociation,
    RespondVodAssociation,
    RequestTimestampSync,
    TimestampSync,
}

export enum VodEditorChannelType {
    Editor,
    Source,
}

interface VodEditorMessagePacket {
    type: VodEditorMessageType
    data: any
}

export class VodEditorContext {
    _uuid: string
    _channel: BroadcastChannel
    _type: VodEditorChannelType
    _vod: VodAssociation | null
    _timeListeners: ((a: Date) => void)[]
    _lastSyncTime: Date | null

    constructor(uuid: string) {
        this._uuid = uuid
        this._channel = new BroadcastChannel(`squadov-vod-editor-${uuid}`)
        this._channel.addEventListener('message', (e: MessageEvent) => { this.handleMessages(e) })
        this._type = VodEditorChannelType.Editor
        this._vod = null
        this._timeListeners = []
        this._lastSyncTime = null
    }

    startSource(v: VodAssociation) {
        this._vod = v
        this._type = VodEditorChannelType.Source
    }

    handleMessages(e: MessageEvent) {
        let respMessage: VodEditorMessagePacket = e.data
        switch (respMessage.type) {
            case VodEditorMessageType.RequestVodAssociation:
                if (this._type == VodEditorChannelType.Source && !!this._vod) {
                    this._channel.postMessage({
                        type: VodEditorMessageType.RespondVodAssociation,
                        data: this._vod,
                    })
                }
            case VodEditorMessageType.RequestTimestampSync:
                if (this._type == VodEditorChannelType.Source && !!this._lastSyncTime) {
                    this._channel.postMessage({
                        type: VodEditorMessageType.TimestampSync,
                        data: this._lastSyncTime,
                    })
                }
            case VodEditorMessageType.TimestampSync:
                this._lastSyncTime = respMessage.data
                this.onSyncTime(respMessage.data)
        }
    }

    requestTimeSync() {
        this._channel.postMessage({
            type: VodEditorMessageType.RequestTimestampSync,
            data: undefined,
        })
    }

    addTimeSyncListener(ln: (a: Date) => void) {
        this._timeListeners.push(ln)
    }

    onSyncTime(t: Date) {
        for (let ln of this._timeListeners) {
            ln(t)
        }
    }

    syncTime(t: Date) {
        this._lastSyncTime = t
        this._channel.postMessage({
            type: VodEditorMessageType.TimestampSync,
            data: t,
        })
    }

    requestVodAssociation(): Promise<VodAssociation> {
        return new Promise((resolve, reject) => {
            let removeTmpListeners = () => {
                this._channel.removeEventListener('message', requestListener)
                this._channel.removeEventListener('messageerror', errorListener)
            }

            let requestListener = (e: MessageEvent) => {
                let respMessage: VodEditorMessagePacket = e.data
                if (respMessage.type == VodEditorMessageType.RespondVodAssociation) {
                    removeTmpListeners()
                    resolve(respMessage.data)
                }
            }

            let errorListener = () => {
                removeTmpListeners()
                reject('Failed to get VOD association for editor.')
            }
            
            this._channel.addEventListener('message', requestListener)
            this._channel.addEventListener('messageerror', errorListener)
            this._channel.postMessage({
                type: VodEditorMessageType.RequestVodAssociation,
                data: undefined,
            })
        })
    }

    close() {
        this._channel.close()
    }
}