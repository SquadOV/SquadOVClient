import { fabric } from 'fabric'
import { BaseDrawAction } from './base'
import { BlurDrawContainer } from '@client/js/draw/blur'

export class DrawActionHistory {
    _canvas: fabric.Canvas
    _blur: BlurDrawContainer

    _currentState: any = {}
    _actions: BaseDrawAction[]
    _actionPtr: number = -1
    _historyCb: (() => void) | null = null

    constructor(canvas: fabric.Canvas, blur: BlurDrawContainer) {
        this._canvas = canvas
        this.reloadState()
        this._blur = blur
        this._actions = []
    }

    setOnHistoryChange(fn: () => void) {
        this._historyCb = fn   
    }

    get hasCurrentAction(): boolean {
        return this._actionPtr >= 0 && this._actionPtr < this._actions.length
    }

    get hasNextAction(): boolean {
        let idx = this._actionPtr + 1
        return idx >= 0 && idx < this._actions.length
    }

    get currentAction(): BaseDrawAction {
        return this._actions[this._actionPtr]
    }

    get savedState(): any {
        return this._currentState
    }

    reloadState() {
        this._currentState = this.state
    }

    get state(): any {
        return this._canvas.toDatalessJSON([
            'blurId',
            'blurAmount',
        ])
    }

    appendDefaultHistory() {
        let action = new BaseDrawAction(this.savedState, this.state)
        this.appendAction(action)
    }

    appendAction(a: BaseDrawAction) {
        if (this._actionPtr < this._actions.length - 1) {
            // We've undo'd back. Now that we're adding another action
            // we need to remove the possibility of redo'ing the stuff we
            // undo'd (pretty sure this is standard behavior).
            this._actions.slice(this._actionPtr + 1)
        }

        this._actions.push(a)
        this._actionPtr += 1
        this._currentState = this.state
        if (!!this._historyCb) {
            this._historyCb()
        }
    }

    stepBackward() {
        if (!this.hasCurrentAction) {
            return
        }

        this.currentAction.undo(this._canvas, this._blur)
        this._actionPtr -= 1
    }

    stepForward() {
        if (!this.hasNextAction) {
            return
        }

        this._actionPtr += 1
        this.currentAction.redo(this._canvas, this._blur)
    }
}