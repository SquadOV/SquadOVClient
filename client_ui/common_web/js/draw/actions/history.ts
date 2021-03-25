import { fabric } from 'fabric'
import { BaseDrawAction } from './base'

export class DrawActionHistory {
    _canvas: fabric.Canvas
    _blurParent: HTMLElement

    _currentState: any = {}
    _actions: BaseDrawAction[]
    _actionPtr: number = -1

    constructor(canvas: fabric.Canvas, blurParent: HTMLElement) {
        this._canvas = canvas
        this._currentState = this.state
        this._blurParent = blurParent
        this._actions = []
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

    get state(): any {
        return this._canvas.toDatalessJSON()
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
    }

    stepBackward() {
        if (!this.hasCurrentAction) {
            return
        }

        this.currentAction.undo(this._canvas, this._blurParent)
        this._actionPtr -= 1
    }

    stepForward() {
        if (!this.hasNextAction) {
            return
        }

        this._actionPtr += 1
        this.currentAction.redo(this._canvas, this._blurParent)
    }
}