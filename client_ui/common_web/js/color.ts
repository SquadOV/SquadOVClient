export interface Color {
    r : number
    g : number
    b : number
}

export function colorToCssString(c: Color): string {
    return `rgb(${c.r}, ${c.g}, ${c.b})`
}

export function getGenericWinColor(): Color {
    // '#4CAF50' 
    return {
        r: 76,
        g: 175,
        b: 80,
    }
}

export function getGenericLossColor() : Color {
    // '#FF5252'
    return {
        r: 255,
        g: 82,
        b: 82,
    }
}

export function getGenericFirstPlaceColor(): Color {
    // '#ffc107'
    return {
        r: 255,
        g: 193,
        b: 7,
    }
}

export function getGenericTopPlaceColor(): Color {
    return getGenericWinColor()
}

export function getGenericBottomPlaceColor(): Color {
    return getGenericLossColor()
}