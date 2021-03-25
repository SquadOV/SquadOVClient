import { BaseDrawTool } from "@client/js/draw/tools/base"
import { BlurDrawContainer } from '@client/js/draw/blur'

export class SelectTool extends BaseDrawTool {
    onActive(c: fabric.Canvas, blur: BlurDrawContainer) {
        super.onActive(c, blur)
        console.log('Activating Select Tool')
        c.selection = true
        for (let o of c.getObjects()) {
            o.selectable = true
        }
    }

    onInactive(c: fabric.Canvas, blur: BlurDrawContainer) {
        super.onInactive(c, blur)
        c.selection = false
        c.discardActiveObject()
        for (let o of c.getObjects()) {
            o.selectable = false
        }
        c.requestRenderAll()
    }
}