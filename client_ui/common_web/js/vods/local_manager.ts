/// #if DESKTOP
import { ipcRenderer } from 'electron'
import path from 'path'
import fs from 'fs'
/// #endif
import { DownloadProgress } from '@client/js/system/download'
import { VodAssociation, VodMetadata, cleanVodAssocationData } from '@client/js/squadov/vod'
import { apiClient } from '@client/js/api'
import { v4 as uuidv4 } from 'uuid'
import { uploadLocalFileToGcs } from '@client/js/gcs'

export type DownloadUploadProgressCb = (progress: number, finish: boolean, error: boolean) => void

class LocalVodIoProgressManager {
    _progress: Map<string, number> = new Map()
    _callbacks: Map<string, DownloadUploadProgressCb[]> = new Map()
    _complete: Set<string> = new Set()

    addCallback(uuid: string, cb: DownloadUploadProgressCb) {
        if (!this._callbacks.has(uuid)) {
            this._callbacks.set(uuid, [])
        }
        this._callbacks.get(uuid)!.push(cb)
        this.notifyProgressFromCache(uuid)
    }

    removeCallback(uuid: string, cb: DownloadUploadProgressCb) {
        if (!this._callbacks.has(uuid)) {
            return
        }

        let idx = this._callbacks.get(uuid)!.findIndex((ele: DownloadUploadProgressCb) => ele === cb)
        if (idx == -1) {
            return
        }

        this._callbacks.get!(uuid)?.splice(idx, 1)
    }

    notifyProgress(uuid: string, val: number) {
        this._progress.set(uuid, val)
        this.notifyProgressFromCache(uuid)
    }

    notifyProgressFromCache(uuid: string) {
        if (!this._callbacks.has(uuid)) {
            return
        }
        let cbs = this._callbacks.get(uuid)!
        let progress = this._progress.get(uuid)
        for (let c of cbs) {
            if (this._complete.has(uuid)) {
                c(1.0, true, false)
            } else if (progress !== undefined) {
                c(progress, false, false)
            }
        }
    }

    finish(uuid: string) {
        this._complete.add(uuid)
        this._progress.delete(uuid)
        this.notifyProgressFromCache(uuid)
    }

    notifyError(uuid: string) {
        this._progress.delete(uuid)
        if (!this._callbacks.has(uuid)) {
            return
        }
        let cbs = this._callbacks.get(uuid)!
        for (let c of cbs) {
            c(0.0, true, true)
        }
    }
}

class LocalVodManager {
    downloads: LocalVodIoProgressManager = new LocalVodIoProgressManager()
    uploads: LocalVodIoProgressManager = new LocalVodIoProgressManager()
    
    constructor() {
    }

    startDownload(uuid: string) {
/// #if DESKTOP
        let progressNotif = (e: any, info: DownloadProgress) => {
            if (info.done) {
                ipcRenderer.removeListener('vod-download-progress', progressNotif)
                if (info.success === false) {
                    this.downloads.notifyError(uuid)
                } else {
                    this.downloads.finish(uuid)
                }
            } else if (info.download !== undefined && info.total !== undefined) {
                if (info.total === 0) {
                    this.downloads.notifyProgress(uuid, 0.0)
                } else {
                    this.downloads.notifyProgress(uuid, info.download / info.total)
                }
            }
        }

        this.downloads.notifyProgress(uuid, 0.0)
        ipcRenderer.send('request-vod-local-download', uuid)
        ipcRenderer.on('vod-download-progress', progressNotif)
/// #endif
    }

    startUpload(vod: VodAssociation, localLoc: string) {
/// #if DESKTOP
        this.uploads.notifyProgress(vod.videoUuid, 0.0)
        setTimeout(async () => {
            try {
                const metadataFname = path.join(path.dirname(localLoc), 'metadata.json')

                if (!fs.existsSync(metadataFname)) {
                    throw 'Could not find metadta filename : ' + metadataFname
                }

                const metadata: VodMetadata = JSON.parse(fs.readFileSync(metadataFname, 'utf8'))
                const uploadPath = await apiClient.getVodUploadPath(vod.videoUuid)

                let progressNotif = async (e: any, info: DownloadProgress) => {
                    if (info.done) {
                        ipcRenderer.removeListener('gcs-upload-progress', progressNotif)
                        if (info.success === false) {
                            throw 'Failed to finish upload: ' + vod.videoUuid
                        }
                    } else if (info.download !== undefined && info.total !== undefined) {
                        if (info.total === 0) {
                            this.uploads.notifyProgress(vod.videoUuid, 0.0)
                        } else {
                            this.uploads.notifyProgress(vod.videoUuid, info.download / info.total)
                        }
                    }
                }
                ipcRenderer.on('gcs-upload-progress', progressNotif)

                const sessionUri = await uploadLocalFileToGcs(localLoc, uploadPath.data, vod.videoUuid)
                let newAssociation: VodAssociation = cleanVodAssocationData(JSON.parse(JSON.stringify(vod)))
                newAssociation.isLocal = false
                await apiClient.associateVod(newAssociation, metadata, sessionUri)
                this.uploads.finish(vod.videoUuid)
            } catch (ex) {
                console.log('Failed to complete upload: ', ex)
                this.uploads.notifyError(vod.videoUuid)
            }
        }, 1)
/// #endif
    }
}

export let manager = new LocalVodManager()