/// #if DESKTOP
import { shell, ipcRenderer } from 'electron'
/// #endif

export function openUrlInBrowser(url: string) {
/// #if DESKTOP
    shell.openExternal(url)
/// #else
    window.open(url, '_blank')
/// #endif
}

export function openPathInNewWindow(path: string) {
/// #if DESKTOP
    ipcRenderer.send('open-path-window', path)
/// #else
    window.open(path, '_blank')
/// #endif
}