import { BaseDrawTool } from "@client/js/draw/tools/base"
import { fabric } from 'fabric'
import { colorAToCssString } from '@client/js/color'
import { BlurDrawContainer } from '@client/js/draw/blur'

export class CircleShapeTool extends BaseDrawTool {
    _activeCircle: fabric.Circle | null = null

    onActive(c: fabric.Canvas, blur: BlurDrawContainer) {
        console.log('Activating Circle Tool')
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
        this._activeCircle = new fabric.Circle({
            selectable: false,
            left: xy.x,
            top: xy.y,
            radius: 0,
            ...this.props,
        })
        this.addObjectToCanvas(this._activeCircle)
    }
    
    onMouseMove(e: fabric.IEvent) {
        if (!this._canvas || !this._activeCircle) {
            return
        }

        let xy = this._canvas.getPointer(e.e)

        // THis is the distance from the top left of the bounding square
        // to the bottom right of the bounding square of the circle so the
        // radius is just half that. We take the maxiumum radius if
        // xDist and yDist are not equivalent.
        let xDist = xy.x - this._activeCircle.left!
        let yDist = xy.y - this._activeCircle.top!
        this._activeCircle.set({
            radius: Math.max(Math.abs(xDist), Math.abs(yDist)) / 2.0,
            originX: xDist > 0 ? 'left' : 'right',
            originY: yDist > 0 ? 'top' : 'bottom',
        })
        this._activeCircle.setCoords()
        this._canvas.requestRenderAll()
    }

    onMouseUp(e: fabric.IEvent) {
        if (!!this._activeCircle) {
            this._history?.appendDefaultHistory()
        }
        this._activeCircle = null
    }

    refreshSettings() {
        if (!this._activeCircle || !this._canvas) {
            return
        }

        this._activeCircle.set(this.props)
        this._canvas.requestRenderAll()
    }
}