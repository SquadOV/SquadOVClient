/// #if DESKTOP
import { ipcRenderer } from 'electron'
import path from 'path'
import fs from 'fs'
/// #endif
import { UploadProgress } from '@client/js/system/upload'
import { DownloadProgress } from '@client/js/system/download'
import { VodAssociation, VodMetadata, cleanVodAssocationData } from '@client/js/squadov/vod'
import { apiClient } from '@client/js/api'
import { v4 as uuidv4 } from 'uuid'
import { uploadLocalFileToCloud } from '@client/js/cloud'
import { IpcResponse } from '../system/ipc'
import { response } from 'express'

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

    uploadQueue: string[] = []
    uploadInProgress: boolean = false
    uploadTicking: boolean = false
    
    constructor() {
        setInterval(async () => {
            if (this.uploadTicking) {
                return
            }
            this.uploadTicking = true
            await this.tickUploadQueue()
            this.uploadTicking = false
        }, 1000)
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

    enqueueUpload(videoUuid: string) {
        this.uploadQueue.push(videoUuid)
    }

    async tickUploadQueue() {
///#if DESKTOP
        if (this.uploadInProgress) {
            return
        }

        let nextVideoUuid: string | undefined = this.uploadQueue.shift()
        if (!nextVideoUuid) {
            return
        }

        try {
            let vod = await apiClient.findVodFromVideoUuid(nextVideoUuid)
            let localResp: IpcResponse<string> = await ipcRenderer.invoke('check-vod-local', nextVideoUuid)

            if (!localResp.success) {
                throw 'No local VOD for uploading found.'
            }

            this.startUpload(vod.data, localResp.data)
        } catch (ex) {
            console.error('Failed to obtain VOD upload data: ', ex)
            return
        }
///#endif
    }

    startUpload(vod: VodAssociation, localLoc: string) {
/// #if DESKTOP
        // No need to upload a VOD that isn't local!
        if (!vod.isLocal) {
            return
        }
        this.uploadInProgress = true
        this.uploads.notifyProgress(vod.videoUuid, 0.0)
        setTimeout(async () => {
            try {
                const metadataFname = path.join(path.dirname(localLoc), 'metadata.json')

                if (!fs.existsSync(metadataFname)) {
                    throw 'Could not find metadta filename : ' + metadataFname
                }

                const metadata: VodMetadata = JSON.parse(fs.readFileSync(metadataFname, 'utf8'))
                const uploadDestination = await apiClient.getVodUploadDestination(vod.videoUuid)

                let progressNotif = async (e: any, info: UploadProgress) => {
                    if (info.done) {
                        ipcRenderer.removeListener('cloud-upload-progress', progressNotif)
                        if (info.success === false) {
                            throw 'Failed to finish upload: ' + vod.videoUuid
                        }
                    } else if (info.upload !== undefined && info.total !== undefined) {
                        if (info.total === 0) {
                            this.uploads.notifyProgress(vod.videoUuid, 0.0)
                        } else {
                            this.uploads.notifyProgress(vod.videoUuid, info.upload / info.total)
                        }
                    }
                }
                ipcRenderer.on('cloud-upload-progress', progressNotif)

                const uploadData = await uploadLocalFileToCloud(localLoc, uploadDestination.data, vod.videoUuid)
                let newAssociation: VodAssociation = cleanVodAssocationData(JSON.parse(JSON.stringify(vod)))
                newAssociation.isLocal = false

                metadata.bucket = uploadDestination.data.bucket
                metadata.sessionId = uploadDestination.data.session
                await apiClient.associateVod(newAssociation, metadata, uploadData.session, uploadData.parts)
                this.uploads.finish(vod.videoUuid)
            } catch (ex) {
                console.error('Failed to complete upload: ', ex)
                this.uploads.notifyError(vod.videoUuid)
            }

            this.uploadInProgress = false
        }, 1)
/// #endif
    }
}

export let manager = new LocalVodManager()