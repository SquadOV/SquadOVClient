import { BaseDrawTool } from "@client/js/draw/tools/base"

export class SelectTool extends BaseDrawTool {
    onActive(c: fabric.Canvas) {
        super.onActive(c)
        console.log('Activating Select Tool')
        c.selection = true
        for (let o of c.getObjects()) {
            o.selectable = true
        }
    }

    onInactive(c: fabric.Canvas) {
        super.onInactive(c)
        c.selection = false
        c.discardActiveObject()
        for (let o of c.getObjects()) {
            o.selectable = false
        }
        c.requestRenderAll()
    }
}