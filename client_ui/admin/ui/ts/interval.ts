export enum Interval {
    Daily,
    Weekly,
    Monthly,
}

export function intervalToNounString(i: Interval): string {
    switch (i) {
        case Interval.Daily:
            return 'Day'
        case Interval.Weekly:
            return 'Week'
        case Interval.Monthly:
            return 'Month'
    }
}

export function intervalToMilliseconds(i: Interval): number {
    switch (i) {
        case Interval.Daily:
            return 86400000
        case Interval.Weekly:
            return 604800000
        case Interval.Monthly:
            return 2592000000
    }
}