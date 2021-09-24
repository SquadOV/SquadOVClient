import format from 'date-fns/format'
import { secondsToTimeString, standardFormatTime, standardFormatDate, standardFormatTimeOnly } from '@client/js/time'

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

export class BaseGraphData {
    _type : string
    _subtype: string
    _name : string
    _group: string
    _groupIcon: string | undefined = undefined
    _groupColor: string | undefined = undefined

    _xLines: XLineMarker[] = []
    _xAreas: XAreaMarker[] = []

    constructor(type : string, subtype: string, name : string) {
        this._type = type
        this._subtype = subtype
        this._name = name
        this._group = 'Default'
    }

    setGroup(g: string) {
        this._group = g
    }

    setGroupStyle(color: string, icon: string) {
        this._groupColor = color
        this._groupIcon = icon
    }

    xFormatter(data: any): string {
        if (this._type == 'time') {
            if (this._subtype == 'date') {
                return standardFormatDate(data)
            } else if (this._subtype == 'time') {
                return standardFormatTimeOnly(data)
            } else {
                return standardFormatTime(data)
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

    
    addXMarkLine(ln: XLineMarker) {
        this._xLines.push(ln)
    }

    addXMarkArea(mk: XAreaMarker) {
        this._xAreas.push(mk)
    }

}