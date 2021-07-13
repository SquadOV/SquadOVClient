/// #if DESKTOP
import { ipcRenderer } from 'electron'
/// #endif

import { VodDestination } from '@client/js/vods/destination'

export interface VodUploadData {
    session: string
    parts: string[] | undefined
}

export function uploadLocalFileToCloud(localFile: string, destination: VodDestination, task: string): Promise<VodUploadData> {
/// #if DESKTOP
    // Dump this task to C++ since that functionality is already there. No need
    // to reinvent the wheel here.
    return ipcRenderer.invoke('request-cloud-upload', {
        task,
        file: localFile,
        destination: destination
    })
///#else
    return new Promise((resolve, reject) => {
        reject('Not supported.')
    })
///#endif
}