import format from 'date-fns/format'

export function standardFormatTime(dt : Date | null) : string {
    if (!dt) {
        return "None"
    }
    return format(dt, 'MMMM do uuuu, h:mm a')
}

export function secondsToTimeString(totalSeconds : number) : string {
    let seconds = totalSeconds % 60
    let minutes = Math.floor(totalSeconds / 60) % 60
    let hours = Math.floor(totalSeconds / 3600)

    let secondsStr = `${seconds}`.padStart(2, '0')
    let minutesStr = `${minutes}`.padStart(2, '0')
    let hoursStr = `${hours}`.padStart(2, '0')

    if (hours > 0) {
        return `${hoursStr}:${minutesStr}:${secondsStr}`
    } else {
        return `${minutesStr}:${secondsStr}`
    }
}