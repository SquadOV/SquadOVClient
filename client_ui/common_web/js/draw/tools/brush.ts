import { fabric } from 'fabric'
import { BaseDrawTool } from "@client/js/draw/tools/base"
import { colorAToCssString } from '@client/js/color'

export class BrushTool extends BaseDrawTool {
    _brush: fabric.PencilBrush | null = null
    _width: number = 8

    get width(): number {
        return this._width
    }

    setWidth(v: number) {
        this._width = v
        this.refreshSettings()
    }

    onActive(c: fabric.Canvas) {
        console.log('Activating Brush Tool')
        if (!this._brush) {
            // @ts-ignore
            this._brush = new fabric.PencilBrush(c)
            this.refreshSettings()
        }

        c.freeDrawingBrush = this._brush
        c.isDrawingMode = true
    }

    onInactive(c: fabric.Canvas) {
        c.isDrawingMode = false
    }

    refreshSettings() {
        if (!this._brush) {
            return
        }
        this._brush.width = this._width
        this._brush.color = colorAToCssString(this._fillColor)
    }
}