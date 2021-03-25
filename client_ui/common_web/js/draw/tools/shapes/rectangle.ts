import { BaseDrawTool } from "@client/js/draw/tools/base"
import { fabric } from 'fabric'
import { colorAToCssString } from '@client/js/color'
import { BlurDrawContainer } from '@client/js/draw/blur'

export class RectangleShapeTool extends BaseDrawTool {
    _activeRectangle: fabric.Rect | null = null

    onActive(c: fabric.Canvas, blur: BlurDrawContainer) {
        console.log('Activating Rectangle Tool')
        super.onActive(c, blur)
    }

    onInactive(c: fabric.Canvas, blur: BlurDrawContainer) {
        super.onInactive(c, blur)
    }

    get props(): any {
        return {
            fill: colorAToCssString(this._fillColor),
            stroke: colorAToCssString(this._outlineColor),
            strokeWidth: this._borderWidth
        }
    }

    onMouseDown(e: fabric.IEvent) {
        if (!this._canvas) {
            return
        }

        let xy = this._canvas.getPointer(e.e)
        // Don't use the refreshSettings function here as that'll cause
        // two events to be added into the history list.
        this._activeRectangle = new fabric.Rect({
            selectable: false,
            left: xy.x,
            top: xy.y,
            width: 0,
            height: 0,
            ...this.props,
        })
        this.addObjectToCanvas(this._activeRectangle)
    }
    
    onMouseMove(e: fabric.IEvent) {
        if (!this._canvas || !this._activeRectangle) {
            return
        }

        let xy = this._canvas.getPointer(e.e)
        let xDist = xy.x - this._activeRectangle.left!
        let yDist = xy.y - this._activeRectangle.top!

        this._activeRectangle.set({
            width: Math.abs(xDist),
            height: Math.abs(yDist),
            originX: xDist > 0 ? 'left' : 'right',
            originY: yDist > 0 ? 'top' : 'bottom',
        })
        this._activeRectangle.setCoords()
        this._canvas.requestRenderAll()
    }

    onMouseUp(e: fabric.IEvent) {
        if (!!this._activeRectangle) {
            this._history?.appendDefaultHistory()
        }
        this._activeRectangle = null
    }

    refreshSettings() {
        if (!this._activeRectangle || !this._canvas) {
            return
        }

        this._activeRectangle.set(this.props)
        this._canvas.requestRenderAll()
    }
}