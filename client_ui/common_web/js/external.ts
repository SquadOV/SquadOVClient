/// #if DESKTOP
import { shell } from 'electron'
/// #endif

export function openUrlInBrowser(url: string) {
/// #if DESKTOP
    shell.openExternal(url)
/// #else
    window.open(url, '_blank')
/// #endif
}