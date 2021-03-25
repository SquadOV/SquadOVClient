import { BaseDrawTool } from "@client/js/draw/tools/base"
import { fabric } from 'fabric'
import { colorAToCssString } from '@client/js/color'

export class TriangleShapeTool extends BaseDrawTool {
    _activeTriangle: fabric.Rect | null = null

    onActive(c: fabric.Canvas) {
        console.log('Activating Triangle Tool')
        super.onActive(c)
    }

    onInactive(c: fabric.Canvas) {
        super.onInactive(c)
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
        this._activeTriangle = new fabric.Triangle({
            selectable: false,
            left: xy.x,
            top: xy.y,
            width: 0,
            height: 0,
            ...this.props,
        })
        this._canvas.add(this._activeTriangle)
    }
    
    onMouseMove(e: fabric.IEvent) {
        if (!this._canvas || !this._activeTriangle) {
            return
        }

        let xy = this._canvas.getPointer(e.e)
        let xDist = xy.x - this._activeTriangle.left!
        let yDist = xy.y - this._activeTriangle.top!

        this._activeTriangle.set({
            width: Math.abs(xDist),
            height: Math.abs(yDist),
            originX: xDist > 0 ? 'left' : 'right',
            originY: yDist > 0 ? 'top' : 'bottom',
        })
        this._activeTriangle.setCoords()
        this._canvas.requestRenderAll()
    }

    onMouseUp(e: fabric.IEvent) {
        this._activeTriangle = null
    }

    refreshSettings() {
        if (!this._activeTriangle || !this._canvas) {
            return
        }

        this._activeTriangle.set(this.props)
        this._canvas.requestRenderAll()
    }
}