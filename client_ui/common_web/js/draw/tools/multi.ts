import { BaseDrawTool } from '@client/js/draw/tools/base'

export class MultiDrawTool extends BaseDrawTool {
    _activeIndex: number = 0
    _subtools: BaseDrawTool[]

    constructor(subtools: BaseDrawTool[]) {
        super()
        this._subtools = subtools
        this._activeIndex = 0
    }

    get activeTool(): BaseDrawTool | null    {
        if (this._activeIndex < 0 || this._activeIndex >= this._subtools.length) {
            return null
        }
        return this._subtools[this._activeIndex]
    }

    setActiveTool(idx: number) {
        if (idx < 0 || idx >= this._subtools.length || !this._canvas) {
            return
        }

        if (!!this.activeTool) {
            this.activeTool.onInactive(this._canvas)
        }

        this._activeIndex = idx

        if (!!this.activeTool) {
            this.activeTool.onActive(this._canvas)
            this.refreshSettings()
        }
    }

    onActive(c: fabric.Canvas) {
        super.onActive(c)

        if (!this.activeTool) {
            return
        }

        this.activeTool.onActive(c)
    }

    onInactive(c: fabric.Canvas) {
        super.onInactive(c)

        if (!this.activeTool) {
            return
        }

        this.activeTool.onInactive(c)
    }

    refreshSettings() {
        if (!this.activeTool) {
            return
        }

        this.activeTool.setFillColor(this._fillColor)
        this.activeTool.setOutlineColor(this._outlineColor)
        this.activeTool.setTextColor(this._textColor)
        this.activeTool.borderWidth = this.borderWidth

        this.activeTool.refreshSettings()
    }

    onMouseDown(e: fabric.IEvent) {
        if (!this.activeTool) {
            return
        }

        this.activeTool.onMouseDown(e)
    }
    
    onMouseMove(e: fabric.IEvent) {
        if (!this.activeTool) {
            return
        }

        this.activeTool.onMouseMove(e)
    }

    onMouseUp(e: fabric.IEvent) {
        if (!this.activeTool) {
            return
        }

        this.activeTool.onMouseUp(e)
    }
}