import { MultiDrawTool } from '@client/js/draw/tools/multi'
import { CircleShapeTool } from '@client/js/draw/tools/shapes/circle'
import { EllipseShapeTool } from '@client/js/draw/tools/shapes/ellipse'
import { RectangleShapeTool } from '@client/js/draw/tools/shapes/rectangle'
import { TriangleShapeTool } from '@client/js/draw/tools/shapes/triangle'

export enum EShapeTool {
    Rectangle,
    Circle,
    Ellipse,
    Triangle
}

export class MultiShapeTool extends MultiDrawTool {
    _borderWidth: number = 0

    constructor() {
        super([
            new RectangleShapeTool(),
            new CircleShapeTool(),
            new EllipseShapeTool(),
            new TriangleShapeTool(),
        ])
    }

    get activeShapeTool(): EShapeTool {
        return this._activeIndex
    }

    set activeShapeTool(s: EShapeTool) {
        this.setActiveTool(s)
    }
}