export interface Color {
    r : number
    g : number
    b : number
}

export interface ColorA extends Color {
    a : number
}

export function colorFromElementTheme(element: Element, key: string): ColorA {
    let value = getComputedStyle(element).getPropertyValue(`--${key}`).trim()
    if (value.startsWith('#')) {
        // Assume the hex is well formed with 6 digits.
        let match = value.match(/\#([0-9A-F][0-9A-F])([0-9A-F][0-9A-F])([0-9A-F][0-9A-F])/i)
        if (!match) {
            console.log('Failed to parse rgba value [hex] from: ', value)
            return {
                r: 0,
                g: 0,
                b: 0,
                a: 0.0
            }    
        }

        return {
            r: parseInt(match[1], 16),
            g: parseInt(match[2], 16),
            b: parseInt(match[3], 16),
            a: 1.0
        }
    } else if (value.startsWith('rgb(')) {
        let match = value.match(/rgb\(\s*?(\d+)\s*?,\s*?(\d+)\s*?,\s*?(\d+)\s*?\)/i)
        if (!match) {
            console.log('Failed to parse rgba value [rgb] from: ', value)
            return {
                r: 0,
                g: 0,
                b: 0,
                a: 0.0
            }    
        }

        return {
            r: parseInt(match[1]),
            g: parseInt(match[2]),
            b: parseInt(match[3]),
            a: 1.0
        }
    } else if (value.startsWith('rgba(')) {
        let match = value.match(/rgba\(\s*?(\d+)\s*?,\s*?(\d+)\s*?,\s*?(\d+)\s*?,\s*?(\d+)\s*?\)/i)
        if (!match) {
            console.log('Failed to parse rgba value [rgba] from: ', value)
            return {
                r: 0,
                g: 0,
                b: 0,
                a: 0.0
            }    
        }

        return {
            r: parseInt(match[1]),
            g: parseInt(match[2]),
            b: parseInt(match[3]),
            a: parseFloat(match[4])
        }
    } else {
        console.log('Failed to parse rgba value from: ', value)
        return {
            r: 0,
            g: 0,
            b: 0,
            a: 0.0
        }
    }
}

export function colorToCssString(c: Color): string {
    return `rgb(${c.r}, ${c.g}, ${c.b})`
}

export function colorAToCssString(c: ColorA): string {
    return `rgba(${c.r}, ${c.g}, ${c.b}, ${c.a})`
}

export function getTransparentColor(): ColorA{ 
    return {
        r: 255,
        g: 255,
        b: 255,
        a: 0.0,
    }
}

export function getRedColor(): ColorA {
    return {
        r: 255,
        g: 0,
        b: 0,
        a: 1.0
    }
}

export function getWhiteColor(): ColorA {
    return {
        r: 255,
        g: 255,
        b: 255,
        a: 1.0
    }
}

export function getBlackColor(): ColorA {
    return {
        r: 0,
        g: 0,
        b: 0,
        a: 1.0
    }
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

export function getStatusMaintenanceColor(): Color {
    return {
        r: 135,
        g: 206,
        b: 250
    }
}

export function getStatusOkColor(): Color {
    return {
        r: 76,
        g: 175,
        b: 80,
    }
}

export function getStatusCriticalColor(): Color {
    return {
        r: 255,
        g: 82,
        b: 82,
    }
}

export function getStatusMajorColor(): Color {
    return {
        r: 255,
        g: 165,
        b: 0,
    }
}

export function getStatusMinorColor(): Color {
    return {
        r: 255,
        g: 193,
        b: 7,
    }
}

export function generateRandomColor(): Color {
    return {
        r: Math.random() * 255,
        g: Math.random() * 255,
        b: Math.random() * 255,
    }
}

const uniqueColorPalette: Color[] = [
    {
        r: 105,
        g: 105,
        b: 105,
    },
    {
        r: 128,
        g: 0,
        b: 0,
    },
    {
        r: 0,
        g: 100,
        b: 0,
    },
    {
        r: 128,
        g: 128,
        b: 0,
    },
    {
        r: 72,
        g: 61,
        b: 139,
    },
    {
        r: 0,
        g: 139,
        b: 139,
    },
    {
        r: 205,
        g: 133,
        b: 63,
    },
    {
        r: 70,
        g: 130,
        b: 180,
    },
    {
        r: 154,
        g: 205,
        b: 50,
    },
    {
        r: 0,
        g: 0,
        b: 139,
    },
    {
        r: 143,
        g: 188,
        b: 143,
    },
    {
        r: 139,
        g: 0,
        b: 139,
    },
    {
        r: 176,
        g: 48,
        b: 96,
    },
    {
        r: 255,
        g: 69,
        b: 0,
    },
    {
        r: 255,
        g: 140,
        b: 0,
    },
    {
        r: 255,
        g: 215,
        b: 0,
    },
    {
        r: 0,
        g: 255,
        b: 0,
    },
    {
        r: 138,
        g: 43,
        b: 226,
    },
    {
        r: 0,
        g: 255,
        b: 127,
    },
    {
        r: 220,
        g: 20,
        b: 60,
    },
    {
        r: 0,
        g: 255,
        b: 255,
    },
    {
        r: 0,
        g: 0,
        b: 255,
    },
    {
        r: 218,
        g: 112,
        b: 214,
    },
    {
        r: 255,
        g: 0,
        b: 255,
    },
    {
        r: 30,
        g: 144,
        b: 255,
    },
    {
        r: 240,
        g: 230,
        b: 140,
    },
    {
        r: 250,
        g: 128,
        b: 114,
    },
    {
        r: 144,
        g: 238,
        b: 144,
    },
    {
        r: 173,
        g: 216,
        b: 230,
    },
    {
        r: 255,
        g: 20,
        b: 147,
    },
    {
        r: 123,
        g: 104,
        b: 238,
    },
    {
        r: 255,
        g: 182,
        b: 193,
    },
]

export function generateUniqueColor(idx: number): Color {
    return uniqueColorPalette[idx % uniqueColorPalette.length]
}