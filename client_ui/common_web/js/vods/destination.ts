export enum VodManagerType {
    FileSystem,
    GCS,
    S3,
};

export interface VodDestination {
    url: string
    bucket: string
    session: string
    loc: VodManagerType 
}