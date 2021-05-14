export interface MfaSecret {
    secret: string
    secretBase32Encoded: string
}

export interface MfaData {
    secret: MfaSecret
    uri: string
}