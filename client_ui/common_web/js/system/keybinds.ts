///#if DESKTOP
import { ipcRenderer } from 'electron'
import { IpcResponse } from '@client/js/system/ipc'

export interface Keybind {
    keys: number[]
    mustHold: boolean
    holdSeconds: number
}

///#endif 
class KeybindsManager {
    _keybindCache: Map<number, string> = new Map()

    async keybindToString(kb: number[]): Promise<string> {
        if (kb.length === 0) {
            return 'No Keybind Set'
        }

///#if DESKTOP
        let uncachedCodes = [
            ...kb.filter((ele: number) => !this._keybindCache.has(ele)),
        ]
        for (let code of uncachedCodes) {
            try {
                let k: IpcResponse<string> = await ipcRenderer.invoke('request-key-code-char', code)
                if (k.success) {
                    this._keybindCache.set(code, k.data)
                } else {
                    throw 'Bad keycode?'
                }
            } catch (ex) {
                console.warn('Failed to request key code: ', code, ex)
                this._keybindCache.set(code, '<ERROR>')
            }
        }
///#endif
        return kb.map((ele: number) => this._keybindCache.get(ele)!).join(' + ')
    }
}

export class KeybindRecordingSession {
    _keybind: number[] = []

    _mouseBind: any
    _keyboardBind: any

    constructor() {
        this._mouseBind = this.addMouse.bind(this)
        window.addEventListener('mouseup', this._mouseBind)

        this._keyboardBind = this.addKeyboardEvent.bind(this)
        window.addEventListener('keydown', this._keyboardBind)
    }

    close() {
        window.removeEventListener('mouseup', this._mouseBind)
        window.removeEventListener('keydown', this._keyboardBind)
    }

    get keySet(): Set<number> {
        return new Set(this._keybind)
    }

    addKeyboardEvent(k: KeyboardEvent) {
        this.addVirtualKeyCode(k.which)
    }

    addVirtualKeyCode(code: number) {
        if (this.keySet.has(code)) {
            return
        }

        this._keybind.push(code)
    }

    addMouse(m: MouseEvent) {
        // This is all windows specific atm.
        switch (m.button) {
            // Mouse wheel -> VK_MBUTTON (0x04)
            case 1:
                this.addVirtualKeyCode(0x04)
                break
            // Right click -> VK_RBUTTON (0x02)
            case 2:
                this.addVirtualKeyCode(0x02)
                break
            // Browser back button -> VK_XBUTTON1 (0x05)
            case 3:
                this.addVirtualKeyCode(0x05)
                break
            // Browser forward button -> VK_XBUTTON2 (0x06)
            case 4:
                this.addVirtualKeyCode(0x06)
                break
            // Ignore left click because otherwise it'd capture the end recording button click.
            // Also ignore other buttons as I'm unsure how they're mapped to virtual key codes.
            default:
                return
        }

        m.preventDefault()
    }

    get keybind(): number[] {
        return this._keybind
    }
}

export default new KeybindsManager()