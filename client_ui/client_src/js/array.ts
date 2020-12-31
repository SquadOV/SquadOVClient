export function generateArrayRange(start: number, end: number) : number[] {
    let arr: number[] = []
    for (let i = start; i < end; ++i) {
        arr.push(i)
    }
    return arr
}