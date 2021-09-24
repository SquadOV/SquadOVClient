const STD_DATETIME_FORMATTER = new Intl.DateTimeFormat([], {
    dateStyle: 'short',
    timeStyle: 'medium',
})

const STD_DATE_FORMATTER = new Intl.DateTimeFormat([], {
    dateStyle: 'short',
})

const STD_TIME_FORMATTER = new Intl.DateTimeFormat([], {
    timeStyle: 'medium',
})

export function standardFormatTime(dt : Date | null) : string {
    if (!dt) {
        return "None"
    }
    return STD_DATETIME_FORMATTER.format(dt)
}

export function standardFormatDate(dt : Date | null) : string {
    if (!dt) {
        return "None"
    }
    return STD_DATE_FORMATTER.format(dt)
}

export function standardFormatTimeOnly(dt : Date | null) : string {
    if (!dt) {
        return "None"
    }
    return STD_TIME_FORMATTER.format(dt)
}

export function secondsToTimeString(totalSeconds : number, forUrl: boolean = false) : string {
    let negFlag = (totalSeconds < 0) ? '-' : ''
    totalSeconds = Math.abs(Math.floor(totalSeconds))
    let seconds = totalSeconds % 60
    let minutes = Math.floor(totalSeconds / 60) % 60
    let hours = Math.floor(totalSeconds / 3600)

    let secondsStr = `${seconds}`.padStart(2, '0')
    let minutesStr = `${minutes}`.padStart(2, '0')
    let hoursStr = `${hours}`.padStart(2, '0')

    if (forUrl) {
        if (hours > 0) {
            return `${negFlag}${hoursStr}h${minutesStr}m${secondsStr}s`
        } else {
            return `${negFlag}${minutesStr}m${secondsStr}s`
        }
    } else {
        if (hours > 0) {
            return `${negFlag}${hoursStr}:${minutesStr}:${secondsStr}`
        } else {
            return `${negFlag}${minutesStr}:${secondsStr}`
        }
    }
}

export function timeStringToSeconds(ts: string | undefined | null): number {
    if (!ts) {
        return 0
    }

    ts = ts.replace('h', ':').replace('m', ':').replace('s', ':')
    let components = ts.split(':')
    if (components.length == 1) {
        // Just seconds
        return parseInt(ts)
    } else {
        let totalSeconds: number = 0
        for (let i = components.length - 1; i >= 0; --i) {
            let num = parseInt(components[i])
            totalSeconds += num * Math.pow(60, components.length - 1 - i)
        }
        return totalSeconds
    }
}

export function millisecondsToTimeString(totalMs : number) : string {
    let negFlag = (totalMs < 0) ? '-' : ''

    totalMs = Math.abs(Math.floor(totalMs))
    let ms = totalMs % 1000
    let seconds = Math.floor(totalMs / 1000) % 60
    let minutes = Math.floor(totalMs / 60000) % 60
    let hours = Math.floor(totalMs / 3600000)

    let msStr = `${ms}`.padStart(3, '0')
    let secondsStr = `${seconds}`.padStart(2, '0')
    let minutesStr = `${minutes}`.padStart(2, '0')
    let hoursStr = `${hours}`.padStart(2, '0')

    if (hours > 0) {
        return `${negFlag}${hoursStr}:${minutesStr}:${secondsStr}.${msStr}`
    } else {
        return `${negFlag}${minutesStr}:${secondsStr}.${msStr}`
    }
}

export function timeStringToMilliseconds(ts: string): number {
    let components = ts.split('.')
    if (components.length === 0) {
        return 0
    }

    let seconds = timeStringToSeconds(components[0])
    let ms = parseInt(components[1])
    return seconds * 1000 + ms
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

export function numDaysAgo(d: Date): number {
    return Math.floor((new Date().getTime() - d.getTime()) / (24 * 60 * 60 * 1000))
}

export function numDaysAgoToString(diffDays: number): string {
    if (diffDays == 0) {
        return 'Today'
    } else if (diffDays == 1) {
        return 'Yesterday'
    } else {
        return `${diffDays} days ago`
    }
}

export function daysAgo(d: Date): string {
    let diffDays = numDaysAgo(d)
    return numDaysAgoToString(diffDays)
}