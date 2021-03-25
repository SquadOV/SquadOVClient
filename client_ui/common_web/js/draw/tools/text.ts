import { BaseDrawTool } from "@client/js/draw/tools/base"
import { fabric } from 'fabric'
import { colorAToCssString } from '@client/js/color'

export class TextTool extends BaseDrawTool {
    _activeText: fabric.Textbox | null = null
    _fontSize: number = 48
    _fontFamily: string = 'Open Sans'
    _bold: boolean = false
    _italic: boolean = false
    _underline: boolean = false
    _align: string = 'left'

    get fontFamily(): string {
        return this._fontFamily
    }

    set fontFamily(v: string) {
        this._fontFamily = v
        this.refreshSettings()
    }

    get fontSize(): number {
        return this._fontSize
    }

    set fontSize(v: number) {
        this._fontSize = v
        this.refreshSettings()
    }

    get bold(): boolean {
        return this._bold
    }

    set bold(v: boolean) {
        this._bold = v
        this.refreshSettings()
    }

    get italic(): boolean {
        return this._italic
    }

    set italic(v: boolean) {
        this._italic = v
        this.refreshSettings()
    }

    get underline(): boolean {
        return this._underline
    }

    set underline(v: boolean) {
        this._underline = v
        this.refreshSettings()
    }

    get align(): string {
        return this._align
    }

    set align(v: string) {
        this._align = v
        this.refreshSettings()
    }

    onActive(c: fabric.Canvas) {
        console.log('Activating Text Tool')
        super.onActive(c)

        if (!!this._canvas) {
            for (let obj of this._canvas.getObjects()) {
                if (obj.get('type') === 'textbox') {
                    obj.selectable = true
                }
            }
        }
    }

    onInactive(c: fabric.Canvas) {
        if (!!this._canvas) {
            for (let obj of this._canvas.getObjects()) {
                if (obj.get('type') === 'textbox') {
                    obj.selectable = false
                }
            }
        }

        super.onInactive(c)

        if (!!this._activeText && !!this._canvas) {
            this._canvas.discardActiveObject()
        }
        this._activeText = null
    }

    onMouseUp(e: fabric.IEvent) {
        if (!this._canvas) {
            return
        }

        let xy = this._canvas.getPointer(e.e)

        // We should be able to modify an existing textbox using the textbox tool.
        // So if we click on an existing textbox, don't create a new one but act like
        // we're modifying the old one!
        let obj = this._canvas.getActiveObject()
        this._activeText = (!!obj && obj.get('type') === 'textbox') ? <fabric.Textbox>obj : null

        if (!this._activeText) {
            // Don't use the reloadSrefreshSettingsettings function here as that'll cause
            // two events to be added into the history list.
            this._activeText = new fabric.Textbox('Made with SquadOV!', {
                top: xy.y,
                left: xy.x,
                ...this.styleObject
            })
            this._canvas.add(this._activeText)
        }

        this._canvas.setActiveObject(this._activeText)
    }

    get styleObject(): any {
        return {
            fontFamily: this._fontFamily,
            fontSize: this._fontSize,
            fontWeight: this.bold ? 700 : 400,
            fontStyle: this.italic ? 'italic' : 'normal',
            underline: this.underline,
            fill: colorAToCssString(this._textColor),
            textAlign: this._align,
        }
    }

    refreshSettings() {
        if (!this._activeText) {
            return
        }

        this._activeText.set(this.styleObject)
        if (!!this._canvas) {
            this._canvas.requestRenderAll()
        }
    }
}