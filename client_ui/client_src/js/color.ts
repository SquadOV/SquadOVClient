export interface Color {
    r : number
    g : number
    b : number
}

export function colorToCssString(c: Color): string {
    return `rgb(${c.r}, ${c.g}, ${c.b})`
}