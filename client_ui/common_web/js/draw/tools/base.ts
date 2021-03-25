import { ColorA, getWhiteColor } from '@client/js/color';
import { fabric } from 'fabric'

export class BaseDrawTool {
    _fillColor: ColorA = getWhiteColor()
    _outlineColor: ColorA = getWhiteColor()
    _textColor: ColorA = getWhiteColor()
    _canvas: fabric.Canvas | null = null

    _mouseDown: (e: fabric.IEvent) => void
    _mouseMove: (e: fabric.IEvent) => void
    _mouseUp: (e: fabric.IEvent) => void

    constructor() {
        this._mouseDown = this.onMouseDown.bind(this)
        this._mouseUp = this.onMouseUp.bind(this)
        this._mouseMove = this.onMouseMove.bind(this)
    }

    onActive(c: fabric.Canvas) {
        this._canvas = c
        this._canvas.on('mouse:down', this._mouseDown)
        this._canvas.on('mouse:up', this._mouseUp)
        this._canvas.on('mouse:move', this._mouseMove)
    }

    onInactive(c: fabric.Canvas) {
        if (!this._canvas) {
            return
        }
        this._canvas.off('mouse:down', this._mouseDown)
        this._canvas.off('mouse:up', this._mouseUp)
        this._canvas.off('mouse:move', this._mouseMove)
        this._canvas = null
    }

    setFillColor(c: ColorA) {
        this._fillColor = c
        this.refreshColors()
    }

    setOutlineColor(c: ColorA) {
        this._outlineColor = c
        this.refreshColors()
    }

    setTextColor(c: ColorA) {
        this._textColor = c
        this.refreshColors()
    }

    refreshColors() {
    }

    onMouseDown(e: fabric.IEvent) {
    }
    
    onMouseMove(e: fabric.IEvent) {
    }

    onMouseUp(e: fabric.IEvent) {
    }
}