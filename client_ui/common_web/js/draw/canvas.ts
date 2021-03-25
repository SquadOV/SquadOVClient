import { fabric } from 'fabric'
import { BaseDrawTool } from '@client/js/draw/tools/base'
import { DrawActionHistory } from '@client/js/draw/actions/history'
import { BaseDrawAction } from '@client/js/draw/actions/base'

export class DrawCanvas {
    canvas: fabric.Canvas
    blurParent: HTMLElement
    tool: BaseDrawTool | null
    history: DrawActionHistory

    _canvasWidth: number = 0
    _canvasHeight: number = 0
    _pauseAutoHistory: boolean = false

    constructor(element: HTMLCanvasElement, blurParent: HTMLElement) {
        this.canvas = new fabric.Canvas(element)
        this.canvas.selection = false
        this.canvas.on('object:added', (e: fabric.IEvent) => this.onObjectAdd(e))
        this.canvas.on('object:modified', (e: fabric.IEvent) => this.onObjectModify(e))
        this.canvas.on('object:removed', (e: fabric.IEvent) => this.onObjectRemove(e))

        this.blurParent = blurParent
        this.tool = null

        this.history = new DrawActionHistory(this.canvas, this.blurParent)
    }

    setWidthHeight(w: number, h: number) {
        // The dimensions of the css and the canvas should be different
        // because the display needs to adapt to the size of the container
        // but the canvas itself should have its width and height stay the same.
        this.canvas.setDimensions({
            width: `${w}px`,
            height: `${h}px`,
        }, {
            cssOnly: true
        })

        let ar = w / h
        let ch = 1080
        let cw = Math.floor(ch * ar)

        if (cw != this._canvasWidth || ch != this._canvasHeight) {
            this.canvas.setDimensions({
                width: cw,
                height: ch,
            }, {
                backstoreOnly: true
            })
        }

        this.canvas.renderAll()
    }

    setActiveTool(tool: BaseDrawTool | null) {
        if (!!this.tool) {
            this.tool.onInactive(this.canvas)
        }

        this.tool = tool

        if (!!this.tool) {
            this.tool.onActive(this.canvas)
        }
    }

    undo() {
        this._pauseAutoHistory = true
        this.history.stepBackward()
        this._pauseAutoHistory = false
    }

    redo() {
        this._pauseAutoHistory = true
        this.history.stepForward()
        this._pauseAutoHistory = false
    }

    deleteSelected() {
        let objs = this.canvas.getActiveObjects()
        this.canvas.discardActiveObject()
        this.canvas.remove(...objs)
    }

    clear() {
        this._pauseAutoHistory = true
        this.canvas.clear()
        while (this.blurParent.lastChild) {
            this.blurParent.removeChild(this.blurParent.lastChild)
        }
        
        this._pauseAutoHistory = false
        this.defaultAutoHistory()
    }

    defaultAutoHistory() {
        if (this._pauseAutoHistory) {
            return
        }
        let action = new BaseDrawAction(this.history.savedState, this.history.state)
        this.history.appendAction(action)
    }

    onObjectAdd(e: fabric.IEvent) {
        this.defaultAutoHistory()
    }

    onObjectModify(e: fabric.IEvent) {
        this.defaultAutoHistory()
    }

    onObjectRemove(e: fabric.IEvent) {
        this.defaultAutoHistory()
    }
}