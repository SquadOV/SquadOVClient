import millify from 'millify'

export function formatBigNumber(v: number): string {
    return millify(v)
}