// When the condition is FALSE, then it always passes the filter (return true).
// When the condition is TRUE, then to pass the filter, cond == filter.
export function applyFilterToCondition(cond: boolean, filter: boolean): boolean {
    if (!cond) {
        return true
    }
    return (cond == filter)
}