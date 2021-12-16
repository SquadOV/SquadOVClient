export interface UploadProgress {
    task: string
    upload?: number
    total?: number
    done: boolean
    success?: boolean
}