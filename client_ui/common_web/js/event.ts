import { Color } from '@client/js/color'

export interface GenericEvent {
    tm: number
    key: string
    value: any
    color: Color
}