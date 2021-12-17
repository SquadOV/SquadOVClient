export function createDisplayStringFromList(input: string[]): string {
    let list = [...input]
    if (list.length > 1) {
        list[list.length - 1] = `and ${list[list.length - 1]}`
    }
    return list.length > 2 ? list.join(', ') : list.join(' ')
}

export function shortenNumberString(val: number): string {
    const siPrefix = ['',  'k', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y']
    const siTier = Math.min(Math.max(Math.floor(Math.log10(Math.abs(val)) / 3), 0), siPrefix.length - 1)
    const prefix = siPrefix[siTier]
    const scale = Math.pow(10, siTier * 3)
    const valScaled = val / scale
    return `${valScaled.toFixed(1)}${prefix}`
}