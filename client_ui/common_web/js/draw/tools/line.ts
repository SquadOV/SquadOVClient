import { BaseDrawTool } from "@client/js/draw/tools/base"
import { fabric } from 'fabric'
import { colorAToCssString } from '@client/js/color'
import { BlurDrawContainer } from '@client/js/draw/blur'

export class LineTool extends BaseDrawTool {
    _activeLine: fabric.Line | null = null
    _width: number = 8

    get width(): number {
        return this._width
    }

    setWidth(v: number) {
        this._width = v
        this.refreshSettings()
    }

    onActive(c: fabric.Canvas, blur: BlurDrawContainer) {
        console.log('Activating Line Tool')
        super.onActive(c, blur)
    }

    onInactive(c: fabric.Canvas, blur: BlurDrawContainer) {
        super.onInactive(c, blur)
    }

    onMouseDown(e: fabric.IEvent) {
        if (!this._canvas) {
            return
        }

        let xy = this._canvas.getPointer(e.e)
        // Don't use the refreshSettings function here as that'll cause
        // two events to be added into the history list.
        this._activeLine = new fabric.Line([xy.x, xy.y, xy.x, xy.y], {
            selectable: false,
            fill: colorAToCssString(this._fillColor),
            stroke: colorAToCssString(this._fillColor),
            strokeWidth: this.width
        })
        this.addObjectToCanvas(this._activeLine)
    }
    
    onMouseMove(e: fabric.IEvent) {
        if (!this._canvas || !this._activeLine) {
            return
        }

        let xy = this._canvas.getPointer(e.e)
        this._activeLine.set({
            'x2': xy.x,
            'y2': xy.y,
        })
        this._activeLine.setCoords()
        this._canvas.requestRenderAll()
    }

    onMouseUp(e: fabric.IEvent) {
        if (!!this._activeLine) {
            this._history?.appendDefaultHistory()
        }
        this._activeLine = null
    }

    refreshSettings() {
        if (!this._activeLine) {
            return
        }

        this._activeLine.fill = colorAToCssString(this._fillColor)
        this._activeLine.stroke = colorAToCssString(this._fillColor)
        this._activeLine.strokeWidth = this.width
    }
}