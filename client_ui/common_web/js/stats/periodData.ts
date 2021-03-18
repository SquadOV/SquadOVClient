import IntervalTree from '@flatten-js/interval-tree'
import * as echarts from 'echarts/lib/echarts.js'
import { BaseGraphData } from '@client/js/stats/graphData'

export class StatTimePeriodSection {
    _name: string
    _start: number
    _end: number
    _icon: string | undefined = undefined
    _iconOpacity: number = 1
    _preferredLayer: number | undefined = undefined
    _color: string | undefined = undefined

    constructor(name: string, start: number, end: number) {
        this._name = name
        this._start = start
        this._end = end
    }

    setIcon(icon: string) {
        this._icon = icon
    }

    setColor(color: string) {
        this._color = color
    }

    setPreferredLayer(layer: number) {
        this._preferredLayer = layer
    }

    setIconOpacity(o: number) {
        this._iconOpacity = o
    }
}

export class StatTimePeriodTrackData {
    _name: string
    _sections: StatTimePeriodSection[] = []

    constructor(name: string) {
        this._name = name
    }

    addSection(s: StatTimePeriodSection) {
        this._sections.push(s)
    }

    compatibleWith(other: StatTimePeriodTrackData): boolean {
        return true
    }
}

class StatTimePeriodLayer {
    _layerIdx: number
    _trackIdx: number
    _sections: StatTimePeriodSection[] = []
    _tree: IntervalTree

    constructor(layerIdx: number, trackIndex: number) {
        this._layerIdx = layerIdx
        this._trackIdx = trackIndex
        this._tree = new IntervalTree()
    }

    canAccept(s: StatTimePeriodSection): boolean {
        let intersect = this._tree.search([s._start, s._end])
        return (intersect.length == 0)
    }

    add(s: StatTimePeriodSection) {
        this._sections.push(s)
        this._tree.insert([s._start, s._end])
    }

    constructEchartData(): any[] {
        return this._sections.map((s: StatTimePeriodSection) => {
            return {
                name: s._name,
                value: [
                    this._layerIdx,
                    this._trackIdx,
                    s._start,
                    s._end,
                    s._end - s._start,
                    s._color,
                    s._icon,
                    s._iconOpacity,
                ],
            }
        })
    }
}

class StatTimePeriodLayerContainer {
    _layers: StatTimePeriodLayer[] = []
    _trackIndex: number

    constructor(t: StatTimePeriodTrackData, trackIndex: number) {
        this._trackIndex = trackIndex
        for (let s of t._sections) {
            let desiredLayer: number = 0
            if (s._preferredLayer !== undefined) {
                desiredLayer = s._preferredLayer
            }

            while (true) {
                while (desiredLayer >= this._layers.length) {
                    this._layers.push(new StatTimePeriodLayer(this._layers.length, this._trackIndex))
                }

                let layer = this._layers[desiredLayer]
                if (layer.canAccept(s)) {
                    layer.add(s)
                    break
                }

                desiredLayer += 1
            }
        }
    }

    get totalLayers(): number {
        return this._layers.length
    }

    constructEchartData(): any[] {
        return this._layers.flatMap((l: StatTimePeriodLayer) => l.constructEchartData())
    }
}

export class StatTimePeriodData extends BaseGraphData {
    _tracks: StatTimePeriodTrackData[] = []

    constructor(name: string) {
        super('value', 'elapsedSeconds', name)
    }

    addTrack(t: StatTimePeriodTrackData) {
        this._tracks.push(t)
    }

    compatibleWith(other: StatTimePeriodData): boolean {
        return this._tracks.length === other._tracks.length
            && this._tracks.every((a: StatTimePeriodTrackData, idx : number) => a.compatibleWith(other._tracks[idx]))
    }

    get layerContainers(): StatTimePeriodLayerContainer[] {
        return this._tracks.map((t: StatTimePeriodTrackData, tIdx: number) => {
            return new StatTimePeriodLayerContainer(t, tIdx)
        })
    }

    get numDisplayLayers(): number {
        let count = 0
        for (let c of this.layerContainers) {
            count += c.totalLayers
        }
        return count
    }

    constructEchartsSeries(heightPerLayer: number, yOffset: number, unit: string, groupIndex: number | undefined = undefined): any {
        // Organize each period data into separate layers where each layer is composed of non-overlapping periods.
        // Note that the general organization of data is
        // PeriodData
        //   --> Track Data
        //      --> Section
        // Each track will have its own group of layers so that period data is not displayed in the layers of other tracks.
        let layers: StatTimePeriodLayerContainer[] = this.layerContainers

        let data: any[] = layers.flatMap((e: StatTimePeriodLayerContainer) => e.constructEchartData())

        // Now we construct the Echarts custom series to render every layer in every track.
        return {
            type: 'custom',
            name: this._name,
            renderItem: (params: any, api: any) => {
                let layerIndex: number = api.value(0)
                let trackIndex: number = api.value(1)
                let displayLayerIndex: number = layerIndex
                for (let i = 0; i < trackIndex; ++i) {
                    displayLayerIndex += layers[i].totalLayers
                }

                let start = api.coord([api.value(2), groupIndex || 0])
                let end = api.coord([api.value(3), groupIndex || 0])

                // The final displayed output is an icon (if one exists) + a rectangle that displays
                // the duration of the period.
                let retGroup: any = {
                    type: 'group',
                    children: []
                }

                let startX = start[0]
                let endX = end[0]
                let width = endX - startX
                let y = yOffset
                if (groupIndex !== undefined) {
                    y += (start[1] - api.size([0, 1])[1] / 2)
                }
                y += heightPerLayer * displayLayerIndex

                let iconStr = api.value(6)
                let iconOpacity = api.value(7)
                if (!!iconStr) {
                    retGroup.children.push({
                        type: 'image',
                        x: startX,
                        y: y,
                        z2: 2,
                        originX: heightPerLayer / 2,
                        originY: heightPerLayer / 2,
                        style: {
                            image: iconStr,
                            width: heightPerLayer,
                            height: heightPerLayer,
                        }
                    })
                }

                let colorStr = api.value(5)
                let rectStyle = api.style()
                rectStyle.fill = colorStr

                retGroup.children.push({
                    type: 'rect',
                    transition: ['shape'],
                    z2: 1,
                    shape: echarts.graphic.clipRectByRect({
                        x: startX,
                        y: y,
                        width: width,
                        height: heightPerLayer
                    }, {
                        x: params.coordSys.x,
                        y: params.coordSys.y,
                        width: params.coordSys.width,
                        height: params.coordSys.height,
                    }),
                    style: rectStyle
                })

                if (iconOpacity < 1) {
                    retGroup.children.push({
                        type: 'rect',
                        transition: ['shape'],
                        z2: 3,
                        shape: echarts.graphic.clipRectByRect({
                            x: startX,
                            y: y,
                            width: Math.max(width, !!iconStr ? heightPerLayer : 0),
                            height: heightPerLayer
                        }, {
                            x: params.coordSys.x,
                            y: params.coordSys.y,
                            width: params.coordSys.width,
                            height: params.coordSys.height,
                        }),
                        style: {
                            fill: 'rgb(0,0,0)',
                            opacity: 1.0 - iconOpacity,
                        }
                    })
                }

                return retGroup
            },
            encode: {
                x: [2, 3],
            },
            data,
            tooltip: {
                formatter: (params: any) => {
                    return `${params.name}: ${params.value[4]} ${unit}`
                }
            }
        }
    }
}