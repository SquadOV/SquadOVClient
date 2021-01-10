export enum BaselineLevel {
    Low,
    Medium,
    High
}

export function baselineToString(b: BaselineLevel) {
    switch (b) {
        case BaselineLevel.Low:
            return "Low Baseline"
        case BaselineLevel.Medium:
            return "Medium Baseline"
        case BaselineLevel.High:
            return "High Baseline"
    }
}

export async function detectComputerBaselineLevel(): Promise<BaselineLevel> {
    // TODO
    return BaselineLevel.High
}