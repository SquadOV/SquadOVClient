import { RectangleShapeTool } from "@client/js/draw/tools/shapes/rectangle"
import { colorAToCssString, getWhiteColor, getTransparentColor } from '@client/js/color'
import { BlurDrawContainer } from '@client/js/draw/blur'

export class BlurTool extends RectangleShapeTool {
    _blurAmount: number = 8

    get blurAmount(): number {
        return this._blurAmount
    }

    set blurAmount(v: number) {
        this._blurAmount = v
    }

    constructor() {
        super()
    }

    get props(): any {
        return {
            fill: colorAToCssString(getTransparentColor()),
            stroke: colorAToCssString(getWhiteColor()),
            strokeWidth: 2,
            lockRotation: true,
            lockSkewingX: true,
            lockSkewingY: true,
        }
    }

    onActive(c: fabric.Canvas, blur: BlurDrawContainer) {
        super.onActive(c, blur)
        console.log('Activating Blur Tool')
    }

    onInactive(c: fabric.Canvas, blur: BlurDrawContainer) {
        super.onInactive(c, blur)
    }

    onMouseMove(e: fabric.IEvent) {
        super.onMouseMove(e)

        if (!this._activeRectangle || !this._blur) {
            return
        }
        this._blur.resyncCanvasObject(this._activeRectangle)
    }

    addObjectToCanvas(o: fabric.Object) {
        super.addObjectToCanvas(o)

        // Create an equivalent div that tracks this object in the blur container.
        if (!!this._blur) {
            this._blur.addObjectForBlur(o, this._blurAmount)
        }
    }

    onMouseUp(e: fabric.IEvent) {
        this._activeRectangle?.set({
            stroke: colorAToCssString(getTransparentColor()),
            strokeWidth: 0,
        })

        if (!!this._canvas) {
            this._canvas.requestRenderAll()
        }
        super.onMouseUp(e)
    }
}