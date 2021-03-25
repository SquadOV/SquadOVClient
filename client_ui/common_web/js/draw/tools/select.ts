import { BaseDrawTool } from "@client/js/draw/tools/base"

export class SelectTool extends BaseDrawTool {
    onActive(c: fabric.Canvas) {
        console.log('Activating Select Tool')
        c.selection = true
        for (let o of c.getObjects()) {
            o.selectable = true
        }
    }

    onInactive(c: fabric.Canvas) {
        c.selection = false
        c.discardActiveObject()
        for (let o of c.getObjects()) {
            o.selectable = false
        }
        c.requestRenderAll()
    }
}