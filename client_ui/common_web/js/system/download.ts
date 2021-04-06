export interface DownloadProgress {
    task: string
    download?: number
    total?: number
    done: boolean
    success?: boolean
}