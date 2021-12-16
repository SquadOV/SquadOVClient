export function createDisplayStringFromList(input: string[]): string {
    let list = [...input]
    if (list.length > 1) {
        list[list.length - 1] = `and ${list[list.length - 1]}`
    }
    return list.length > 2 ? list.join(', ') : list.join(' ')
}