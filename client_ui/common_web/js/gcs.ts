/// #if DESKTOP
import { ipcRenderer } from 'electron'
/// #endif

export function uploadLocalFileToGcs(localFile: string, gcsUri: string, task: string): Promise<string> {
/// #if DESKTOP
    // Dump this task to C++ since that functionality is already there. No need
    // to reinvent the wheel here.
    return ipcRenderer.invoke('request-gcs-upload', {
        task,
        file: localFile,
        uri: gcsUri
    })
///#else
    return new Promise((resolve, reject) => {
        reject('Not supported.')
    })
///#endif
}