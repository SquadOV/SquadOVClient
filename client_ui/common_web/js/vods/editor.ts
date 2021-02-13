/// #if DESKTOP
import { ipcRenderer } from 'electron'
/// #endif

export function openVodEditingWindow(videoUuid: string) {
/// #if DESKTOP
    ipcRenderer.invoke('open-vod-editor', videoUuid)
/// #else

/// #endif
}