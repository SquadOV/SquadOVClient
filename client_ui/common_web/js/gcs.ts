/// #if DESKTOP
import { ipcRenderer } from 'electron'
import { v4 as uuidv4 } from 'uuid'
/// #endif

export function uploadLocalFileToGcs(localFile: string, gcsUri: string): Promise<string> {
    // Dump this task to C++ since that functionality is already there. No need
    // to reinvent the wheel here.
    return ipcRenderer.invoke('request-gcs-upload', {
        task: uuidv4(),
        file: localFile,
        uri: gcsUri
    })
}