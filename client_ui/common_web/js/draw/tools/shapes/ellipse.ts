import { BaseDrawTool } from "@client/js/draw/tools/base"
import { fabric } from 'fabric'
import { colorAToCssString } from '@client/js/color'

export class EllipseShapeTool extends BaseDrawTool {
    _activeEllipse: fabric.Ellipse | null = null

    onActive(c: fabric.Canvas) {
        console.log('Activating Ellipse Tool')
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
        this._activeEllipse = new fabric.Ellipse({
            selectable: false,
            left: xy.x,
            top: xy.y,
            rx: 0,
            ry: 0,
            ...this.props,
        })
        this._canvas.add(this._activeEllipse)
    }
    
    onMouseMove(e: fabric.IEvent) {
        if (!this._canvas || !this._activeEllipse) {
            return
        }

        let xy = this._canvas.getPointer(e.e)

        // THis is the distance from the top left of the bounding square
        // to the bottom right of the bounding square of the Ellipse so the
        // radius is just half that.
        let xDist = xy.x - this._activeEllipse.left!
        let yDist = xy.y - this._activeEllipse.top!
        this._activeEllipse.set({
            rx: Math.abs(xDist) / 2.0,
            ry: Math.abs(yDist) / 2.0,
            originX: xDist > 0 ? 'left' : 'right',
            originY: yDist > 0 ? 'top' : 'bottom',
        })
        this._activeEllipse.setCoords()
        this._canvas.requestRenderAll()
    }

    onMouseUp(e: fabric.IEvent) {
        this._activeEllipse = null
    }

    refreshSettings() {
        if (!this._activeEllipse || !this._canvas) {
            return
        }

        this._activeEllipse.set(this.props)
        this._canvas.requestRenderAll()
    }
}