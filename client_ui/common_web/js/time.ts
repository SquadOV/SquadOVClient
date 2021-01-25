import format from 'date-fns/format'

export function standardFormatTime(dt : Date | null) : string {
    if (!dt) {
        return "None"
    }
    return format(dt, 'MM/dd/yyyy, h:mm a')
}

export function secondsToTimeString(totalSeconds : number) : string {
    totalSeconds = Math.floor(totalSeconds)
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
    totalMs = Math.floor(totalMs)
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

export function dateMin(a: Date, b: Date): Date {
    if (a < b) {
        return a
    } else if (a > b) {
        return b
    }
    return a
}

export function dateMax(a: Date, b: Date): Date {
    if (a < b) {
        return b;
    } else if (a > b) {
        return a
    }
    return b
}

export function dateClamp(x: Date, min: Date, max: Date): Date {
    return dateMin(dateMax(x, min), max)
}

export function dateInRange(x: Date, min: Date, max: Date): boolean {
    return (x >= min && x <= max)
}

export function dateRangeIntersects(aMin: Date, aMax: Date, bMin: Date, bMax: Date): boolean {
    return dateInRange(aMin, bMin, bMax) ||
        dateInRange(aMax, bMin, bMax) ||
        dateInRange(bMin, aMin, aMax) ||
        dateInRange(bMax, aMin, aMax)
}

export function daysAgo(d: Date): string {
    let diffDays = Math.floor((new Date().getTime() - d.getTime()) / (24 * 60 * 60 * 1000))
    if (diffDays == 0) {
        return 'Today'
    } else if (diffDays == 1) {
        return 'Yesterday'
    } else {
        return `${diffDays} days ago`
    }
}