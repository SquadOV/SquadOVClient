export class BaseDrawAction {
    _pre: any
    _post: any

    constructor(pre: any, post: any) {
        this._pre = pre
        this._post = post
    }

    undo(canvas: fabric.Canvas, blurParent: HTMLElement) {
        canvas.loadFromJSON(this._pre, () => {})
    }

    redo(canvas: fabric.Canvas, blurParent: HTMLElement) {
        canvas.loadFromJSON(this._post, () => {})
    }
}