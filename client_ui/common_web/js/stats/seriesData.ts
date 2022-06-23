import { StatTimePeriodData } from '@client/js/stats/periodData'
import { BaseGraphData } from '@client/js/stats/graphData'

export interface BorderStyle {
    width: number
    color: string
}

export interface LineStyle {
    type?: string | undefined
    color?: string | undefined
    border?: BorderStyle | undefined
}

export class StatXYSeriesData extends BaseGraphData {
    _x : any[]
    _y : any[]

    _style: LineStyle | undefined = undefined
    _symbol: string | undefined = undefined
    _allGroups: boolean = false

    _overlayPeriods: StatTimePeriodData[] = []

    get hasStyle(): boolean {
        return !!this._style
    }

    get echartsLineStyle(): any {
        let style: any = { }
        if (!!this._style?.type) {
            style.type = this._style.type
        }

        if (!!this._style?.color) {
            style.color = this._style.color
        }
        return style
    }

    get echartsItemStyle(): any {
        let style: any = { }
        if (!!this._style?.color) {
            style.color = this._style.color
        }

        if (!!this._style?.border) {
            style.borderColor = this._style.border.color
            style.borderWidth = this._style.border.width
        }
        return style
    }

    compatibleWith(other: StatXYSeriesData): boolean {
        let base = this._type === other._type && this._subtype === other._subtype && this._overlayPeriods.length === other._overlayPeriods.length
        if (!base) {
            return false
        }

        for (let i = 0; i < this._overlayPeriods.length; ++i) {
            if (this._overlayPeriods[i]._tracks.length != other._overlayPeriods[i]._tracks.length) {
                return false
            }
        }

        return true
    }

    setStyle(style: LineStyle | undefined) {
        this._style = style
    }

    setSymbol(symbol: string | undefined) {
        this._symbol = symbol
    }

    setAllGroups(v: boolean) {
        this._allGroups = v
    }

    get isForAllGroups(): boolean {
        return this._allGroups
    }

    setXY(x: any[], y: any[]) {
        this._x = x
        this._y = y
    }

    addOverlayPeriod(p: StatTimePeriodData) {
        this._overlayPeriods.push(p)
    }

    get showSymbol(): boolean {
        return !!this._symbol
    }

    constructor(x : any[], y : any[], type : string, subtype: string, name : string) {
        super(type, subtype, name)
        if (x.length != y.length) {
            console.warn('Error in series data: X-Y length mismatch.')
        }
        this._x = x
        this._y = y
        this._xLines = []
        this._xAreas = []
    }

    get reversed() : boolean {
        if (this._type != 'time') {
            return false
        }

        if (this.length <= 1) {
            return false
        }

        return this._x[0] > this._x[1]
    }

    get length() : number {
        return this._x.length
    }
}