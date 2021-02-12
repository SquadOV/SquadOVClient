export function formatRoundTime(ms : number) : string {
    const totalSeconds = Math.floor(ms / 1000.0)
    const minutes = `${Math.floor(totalSeconds / 60.0)}`
    const seconds = `${totalSeconds % 60}`.padStart(2, '0')
    return `${minutes}:${seconds}`
}