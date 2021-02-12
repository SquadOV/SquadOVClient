export function generateArrayRange(start: number, end: number) : number[] {
    let arr: number[] = []
    for (let i = start; i < end; ++i) {
        arr.push(i)
    }
    return arr
}

export function generateSteppedArrayRange(start: number, end: number, step: number) : number[] {
    let arr: number[] = []
    for (let i = start; i < end; i += step) {
        arr.push(i)
    }
    return arr
}