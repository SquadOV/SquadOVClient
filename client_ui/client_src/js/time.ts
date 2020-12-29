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

export function millisecondsToTimeString(totalMs : number) : string {
    let ms = totalMs % 1000
    let seconds = Math.floor(totalMs / 1000) % 60
    let minutes = Math.floor(totalMs / 60000) % 60
    let hours = Math.floor(totalMs / 3600000)

    let msStr = `${ms}`.padStart(3, '0')
    let secondsStr = `${seconds}`.padStart(2, '0')
    let minutesStr = `${minutes}`.padStart(2, '0')
    let hoursStr = `${hours}`.padStart(2, '0')

    if (hours > 0) {
        return `${hoursStr}:${minutesStr}:${secondsStr}.${msStr}`
    } else {
        return `${minutesStr}:${secondsStr}.${msStr}`
    }
}