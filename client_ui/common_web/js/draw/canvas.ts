import { fabric } from 'fabric'
import { BaseDrawTool } from '@client/js/draw/tools/base'
import { DrawActionHistory } from '@client/js/draw/actions/history'
import { BaseDrawAction } from '@client/js/draw/actions/base'
import { BlurDrawContainer } from '@client/js/draw/blur'

export class DrawCanvas {
    canvas: fabric.Canvas
    blur: BlurDrawContainer
    tool: BaseDrawTool | null
    history: DrawActionHistory

    _canvasWidth: number = 0
    _canvasHeight: number = 0
    _pauseAutoHistory: boolean = false

    constructor(element: HTMLCanvasElement, blurParent: HTMLElement) {
        this.canvas = new fabric.Canvas(element)
        this.canvas.selection = false
        this.canvas.on('path:created', (e: fabric.IEvent) => this.defaultAutoHistory())
        this.canvas.on('object:modified', (e: fabric.IEvent) => this.defaultAutoHistory())

        this.blur = new BlurDrawContainer(this.canvas, blurParent)
        this.tool = null

        this.history = new DrawActionHistory(this.canvas, this.blur)
    }

    loadState(state: any) {
        this.canvas.loadFromJSON(state, () => {})
        this.blur.resyncAll()
        this.history.reloadState()
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

        let ar = w * 1.0 / h
        let ch = 1080
        let cw = Math.floor(ch * ar)

        if (cw != this._canvasWidth || ch != this._canvasHeight) {
            this.canvas.setDimensions({
                width: cw,
                height: ch,
            }, {
                backstoreOnly: true
            })

            this._canvasWidth = cw
            this._canvasHeight = ch
        }

        this.canvas.renderAll()
    }

    setActiveTool(tool: BaseDrawTool | null) {
        if (!!this.tool) {
            this.tool.onInactive(this.canvas, this.blur)
        }

        this.tool = tool

        if (!!this.tool) {
            this.tool.setHistory(this.history)
            this.tool.onActive(this.canvas, this.blur)
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
        this.defaultAutoHistory()
    }

    clear() {
        this._pauseAutoHistory = true
        this.canvas.clear()
        this.blur.clear()
        
        this._pauseAutoHistory = false
        this.defaultAutoHistory()
    }

    defaultAutoHistory() {
        if (this._pauseAutoHistory) {
            return
        }

        this.history.appendDefaultHistory()
    }
}