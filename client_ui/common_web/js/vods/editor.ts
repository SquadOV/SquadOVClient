import { VodAssociation } from '@client/js/squadov/vods'

/// #if DESKTOP
import { ipcRenderer } from 'electron'
import { Message } from 'protobufjs'
/// #endif

export function openVodEditingWindow(videoUuid: string) {
/// #if DESKTOP
    ipcRenderer.invoke('open-vod-editor', videoUuid)
/// #else

/// #endif
}

enum VodEditorMessageType {
    RequestVodAssociation
}

export class VodEditorContext {
    _uuid: string
    _channel: BroadcastChannel

    constructor(uuid: string) {
        this._uuid = uuid
        this._channel = new BroadcastChannel(`squadov-vod-editor-${uuid}`)
        this._channel.addEventListener('message', (e: MessageEvent) => { this.handleMessages(e) })
    }

    handleMessages(e: MessageEvent) {
        
    }

    requestVodAssociation(): Promise<VodAssociation> {
        return new Promise((resolve, reject) => {
            let requestListener = (e: Message) => {
            }
            this._channel.addEventListener('message', requestListener)
            this._channel.postMessage(VodEditorMessageType.RequestVodAssociation)
        })
    }

    close() {
        this._channel.close()
    }
}