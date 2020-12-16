import { Long } from 'long'

export function longNumberToNumber(x: Long | number): number {
    if (typeof x === 'number') {
        return x
    } else {
        return x.toNumber()
    }
}