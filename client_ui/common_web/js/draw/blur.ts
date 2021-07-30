import { fabric } from 'fabric'
import { v4 as uuidv4 } from 'uuid'

export class BlurDrawContainer {
    _canvas: fabric.Canvas
    _blurParent: HTMLElement
    _blurElements: Map<string, HTMLElement>

    constructor(canvas: fabric.Canvas, blurParent: HTMLElement) {
        this._canvas = canvas
        this._canvas.on('object:moving', (e: fabric.IEvent) => this.resyncCanvasObject(e.target!))
        this._canvas.on('object:scaling', (e: fabric.IEvent) => this.resyncCanvasObject(e.target!))
        this._blurParent = blurParent
        this._blurElements = new Map()
    }

    addObjectForBlur(o: fabric.Object, blurAmount: number) {
        let blurId = `blur-${uuidv4()}`
        //@ts-ignore
        o.blurId = blurId
        //@ts-ignore
        o.blurAmount = blurAmount
        this.resyncCanvasObject(o)
    }

    resyncAll() {
        // In this case, the canvas is the authoritative state so we only sync
        // an object if it still exists in the canvas.
        let objectsForSync: Map<string, fabric.Object> = new Map()
        for (let obj of this._canvas.getObjects()) {
            //@ts-ignore
            let blurId = obj.blurId
            if (!!blurId) {
                objectsForSync.set(blurId, obj)
            }
        }

        let elesToRemove: string[] = []
        for (let [blurId, _] of this._blurElements) {
            if (objectsForSync.has(blurId)) {
                let obj = objectsForSync.get(blurId)!
                this.resyncCanvasObject(obj)
                objectsForSync.delete(blurId)
            } else {
                elesToRemove.push(blurId)
            }
        }

        for (let blurId of elesToRemove) {
            let ele = this._blurElements.get(blurId)
            if (!ele) {
                continue
            }
            this._blurElements.delete(blurId)
            this._blurParent.removeChild(ele)
        }

        for (let [_, obj] of objectsForSync) {
            this.resyncCanvasObject(obj)
        }
    }

    resyncCanvasObject(o: fabric.Object) {
        //@ts-ignore
        let blurId = o.blurId

        //@ts-ignore
        let blurAmount = o.blurAmount

        let originX = o.originX || 'left'
        let originY = o.originY || 'top'

        if (!blurId || blurAmount === undefined) {
            return
        }

        let blurEle = this._blurElements.get(blurId)
        if (!blurEle) {
            blurEle = document.createElement('div')
            blurEle.id = blurId
            //@ts-ignore
            blurEle.style.backdropFilter = `blur(${blurAmount}px)`
            blurEle.style.position = 'absolute'
            this._blurElements.set(blurId, blurEle)
            this._blurParent.appendChild(blurEle)
        }

        // Need to convert the canvas object's absolute
        // positions/width/height to percentages as we need
        // to be able to handle when the width/height of the
        // blur parent div changes.
        let canvasWidth = this._canvas.getElement().width
        let canvasHeight = this._canvas.getElement().height

        let realWidth = o.width! * o.scaleX!
        let realHeight = o.height! * o.scaleY!

        let width = realWidth / canvasWidth * 100
        let height = realHeight / canvasHeight * 100
        let left = ((originX == 'left') ? o.left! : o.left! - realWidth) / canvasWidth * 100
        let top = ((originY == 'top') ? o.top! : o.top! - realHeight) / canvasHeight * 100

        blurEle.style.left = `${left}%`
        blurEle.style.top = `${top}%`
        blurEle.style.width = `${width}%`
        blurEle.style.height = `${height}%`
    }

    clear() {
        while (this._blurParent.lastChild) {
            this._blurParent.removeChild(this._blurParent.lastChild)
        }
        this._blurElements.clear()
    }
}