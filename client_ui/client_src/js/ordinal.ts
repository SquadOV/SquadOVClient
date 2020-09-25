export function getOrdinalSuffix(n : number) : string {
    // Really only mentally tested this up until ~40.
    const cmp = n % 100
    if (cmp >= 10 && cmp <= 20) {
        return 'th'
    } else {
        const digit = n % 10
        if (digit == 1) {
            return 'st'
        } else if (digit == 2) {
            return 'nd'
        } else if (digit == 3) {
            return 'rd'
        } else {
            return 'th'
        }
    }
}

export function getOrdinal(n : number) : string {
    return `${n}${getOrdinalSuffix(n)}`
}