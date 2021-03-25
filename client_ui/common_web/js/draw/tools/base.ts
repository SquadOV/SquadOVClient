import { ColorA, getWhiteColor } from '@client/js/color';
import { fabric } from 'fabric'
import { BlurDrawContainer } from '@client/js/draw/blur'
import { DrawActionHistory } from '@client/js/draw/actions/history'

export class BaseDrawTool {
    _fillColor: ColorA = getWhiteColor()
    _outlineColor: ColorA = getWhiteColor()
    _textColor: ColorA = getWhiteColor()
    _canvas: fabric.Canvas | null = null
    _blur: BlurDrawContainer | null = null
    _history: DrawActionHistory | null = null
    _borderWidth: number = 2

    _mouseDown: (e: fabric.IEvent) => void
    _mouseMove: (e: fabric.IEvent) => void
    _mouseUp: (e: fabric.IEvent) => void

    constructor() {
        this._mouseDown = this.onMouseDown.bind(this)
        this._mouseUp = this.onMouseUp.bind(this)
        this._mouseMove = this.onMouseMove.bind(this)
    }

    setHistory(h: DrawActionHistory) {
        this._history = h
    }

    onActive(c: fabric.Canvas, blur: BlurDrawContainer) {
        this._canvas = c
        this._canvas.on('mouse:down', this._mouseDown)
        this._canvas.on('mouse:up', this._mouseUp)
        this._canvas.on('mouse:move', this._mouseMove)
        this._blur = blur
    }

    onInactive(c: fabric.Canvas, blur: BlurDrawContainer) {
        if (!this._canvas) {
            return
        }
        this._canvas.off('mouse:down', this._mouseDown)
        this._canvas.off('mouse:up', this._mouseUp)
        this._canvas.off('mouse:move', this._mouseMove)
        this._canvas = null
        this._blur = null
    }

    addObjectToCanvas(o: fabric.Object) {
        if (!this._canvas) {
            return
        }
        this._canvas.add(o)
    }

    get borderWidth(): number {
        return this._borderWidth
    }

    set borderWidth(v: number) {
        this._borderWidth = v
        this.refreshSettings()
    }

    setFillColor(c: ColorA) {
        this._fillColor = c
        this.refreshSettings()
    }

    setOutlineColor(c: ColorA) {
        this._outlineColor = c
        this.refreshSettings()
    }

    setTextColor(c: ColorA) {
        this._textColor = c
        this.refreshSettings()
    }

    refreshSettings() {
    }

    onMouseDown(e: fabric.IEvent) {
    }
    
    onMouseMove(e: fabric.IEvent) {
    }

    onMouseUp(e: fabric.IEvent) {
    }
}