import { BaseDrawTool } from '@client/js/draw/tools/base'
import { BlurDrawContainer } from '@client/js/draw/blur'
import { DrawActionHistory } from '@client/js/draw/actions/history'

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

    setHistory(h: DrawActionHistory) {
        super.setHistory(h)

        for (let s of this._subtools) {
            s.setHistory(h)
        }
    }

    setActiveTool(idx: number) {
        if (idx < 0 || idx >= this._subtools.length || !this._canvas || !this._blur) {
            return
        }

        if (!!this.activeTool) {
            this.activeTool.onInactive(this._canvas, this._blur)
        }

        this._activeIndex = idx

        if (!!this.activeTool) {
            this.activeTool.onActive(this._canvas, this._blur)
            this.refreshSettings()
        }
    }

    onActive(c: fabric.Canvas, blur: BlurDrawContainer) {
        super.onActive(c, blur)

        if (!this.activeTool) {
            return
        }

        this.activeTool.onActive(c, blur)
    }

    onInactive(c: fabric.Canvas, blur: BlurDrawContainer) {
        super.onInactive(c, blur)

        if (!this.activeTool) {
            return
        }

        this.activeTool.onInactive(c, blur)
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
}