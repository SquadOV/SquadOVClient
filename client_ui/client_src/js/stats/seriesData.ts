import { assert } from 'console'

export class StatXYSeriesData {
    _x : any[]
    _y : any[]
    _type : string
    _name : string

    constructor(x : any[], y : any[], type : string, name : string) {
        assert(x.length == y.length)
        this._x = x
        this._y = y
        this._type = type
        this._name = name
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