export function formatGoldAmount(gold: number): string {
    return `${(gold / 1000.0).toFixed(1)}k`
}