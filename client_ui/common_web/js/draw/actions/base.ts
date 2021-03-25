import { BlurDrawContainer } from '@client/js/draw/blur'

export class BaseDrawAction {
    _pre: any
    _post: any

    constructor(pre: any, post: any) {
        this._pre = pre
        this._post = post
    }

    undo(canvas: fabric.Canvas, blurParent: BlurDrawContainer) {
        canvas.loadFromJSON(this._pre, () => {})
        blurParent.resyncAll()
    }

    redo(canvas: fabric.Canvas, blurParent: BlurDrawContainer) {
        canvas.loadFromJSON(this._post, () => {})
        blurParent.resyncAll()
    }
}