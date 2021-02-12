import { secondsToTimeString } from '@client/js/time'
import format from 'date-fns/format'

export interface XLineMarker {
    x: any
    name: string
    symbol: string
    colorOverride?: string
}

export interface XAreaMarker {
    start: any,
    end: any,
    name: string
}

export class StatXYSeriesData {
    _x : any[]
    _y : any[]
    _type : string
    _subtype: string
    _name : string

    _xLines: XLineMarker[]
    _xAreas: XAreaMarker[]

    compatibleWith(other: StatXYSeriesData): boolean {
        return this._type === other._type && this._subtype === other._subtype
    }

    addXMarkLine(ln: XLineMarker) {
        this._xLines.push(ln)
    }

    addXMarkArea(mk: XAreaMarker) {
        this._xAreas.push(mk)
    }

    constructor(x : any[], y : any[], type : string, subtype: string, name : string) {
        if (x.length != y.length) {
            console.log('Error in series data: X-Y length mismatch.')
        }
        this._x = x
        this._y = y
        this._type = type
        this._subtype = subtype
        this._name = name
        this._xLines = []
        this._xAreas = []
    }
    
    xFormatter(data: any): string {
        if (this._type == 'time') {
            if (this._subtype == 'date') {
                return format(data, 'MMMM do uuuu')
            } else if (this._subtype == 'time') {
                return format(data, 'h:mm a')
            } else {
                return format(data, 'MMMM do uuuu, h:mm a')
            }
        } else if (this._type == 'value') {
            if (this._subtype == 'elapsedSeconds') {
                return secondsToTimeString(data)
            } else {
                return data
            }
        } else {
            return data
        }
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