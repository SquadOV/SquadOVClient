<template>
   <div
        ref="parent"
        class="parent-div"
        :style="style"
        @mouseenter="startSeeking"
        @mouseleave="clearMouseMoveHandlers"
        @mouseup="clearMouseMoveHandlers"
        @mousemove="onMouseMove"
        @click="changeCurrent"
    >
        <div ref="inner" class="bar-div" :style="progressBarStyle" v-if="!disableSeeking">
        </div>

        <div ref="seekTick" class="seek-div" :style="seekDivStyle" v-if="seeking && !disableSeeking">
            <slot name="tick" v-bind:tick="seekingTime">
                {{ seekingTime }}
            </slot>
        </div>

        <div class="clip-div" v-if="hasClipHandles" :style="clipBarStyle" @mousedown="startMove" @mouseup="endMove">
            <div
                class="clip-start-div"
                @mousedown.stop="moveStart = true"
                @mouseup="moveStart = false"
            ></div>
            <div
                class="clip-end-div"
                @mousedown.stop="moveEnd = true"
                @mouseup="moveEnd = false"
            ></div>
        </div>

        <div class="full-parent-height tick-parent" :style="parentTickDivStyle">
            <div
                class="tick-div major-tick"
                v-for="(tick, idx) in majorTicks"
                :key="`major-${idx}`"
                :style="tickDivStyle(tick)"
            >
                <slot name="tick" v-bind:tick="tick">
                    {{ tick }}
                </slot>

                <div class="d-flex justify-center inner-tick">
                    <div class="tick tick-mark"></div>
                    <div class="tick"></div>
                </div>
            </div>

            <div
                v-for="(tick, idx) in minorTicks"
                :key="`minor-${idx}`"
                class="tick-div minor-tick d-flex justify-center"
                :style="tickDivStyle(tick)"
            >
                <div class="tick tick-mark"></div>
                <div class="tick"></div>
            </div>

            <div
                class="tick-div event-tick event-div"
                v-for="(eve, idx) in inputEvents"
                :key="`event-${idx}`"
                :style="tickDivStyle(eve.tm)"
            >
                <slot name="event" v-bind:event="eve">
                </slot>

                <slot :name="eve.key" v-bind:event="eve">
                </slot>

                <div class="d-flex justify-center inner-tick">
                    <div class="tick" :style="eventTickMark(eve)"></div>
                    <div class="tick"></div>
                </div>
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component, { mixins } from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { GenericEvent} from '@client/js/event'
import { generateSteppedArrayRange } from '@client/js/array'
import { colorToCssString } from '@client/js/color'
import CommonComponent from '@client/vue/CommonComponent'

@Component
export default class GenericMatchTimeline extends mixins(CommonComponent) {
    @Prop({required: true})
    start!: number

    @Prop({required: true})
    end!: number

    @Prop({default: 100})
    majorTickEvery!: number

    @Prop({default: 10})
    intervalTicks!: number

    @Prop({default: 64})
    height!: number

    @Prop({type: Boolean, default: false})
    fill!: boolean

    @Prop({default: 0})
    current!: number

    @Prop({type: Array, required: true})
    inputEvents!: GenericEvent[]

    @Prop()
    clipStartHandle!: number

    @Prop()
    clipEndHandle!: number

    @Prop({type: Boolean, default: false })
    disableSeeking!: boolean

    $refs!: {
        parent: HTMLElement
        inner: HTMLElement
        seekTick: HTMLElement
    }
    seeking: boolean = false
    seekingTime: number = 0
    lastMouseMove: Date = new Date()

    moveStart: boolean = false
    moveEnd: boolean = false
    diffStart: number = 0
    diffEnd: number = 0

    tooltipX: number = 0
    tooltipY: number = 0

    get hasClipHandles(): boolean {
        return this.clipStartHandle !== undefined && this.clipEndHandle !== undefined
    }

    get clipBarStyle(): any {
        if (!this.hasClipHandles) {
            return {}
        }
        let clipStartPercentage = this.computePercentage(this.clipStartHandle)
        let clipEndPercentage = this.computePercentage(this.clipEndHandle)
        return {
            'width': `calc(${clipEndPercentage - clipStartPercentage}%)`,
            'left': `calc(${clipStartPercentage}%)`,
            'cursor': 'pointer',
        }
    }

    startMove(e: MouseEvent) {
        let currentTime = this.computeTimeFromMouseEvent(e)
        this.diffStart = this.clipStartHandle - currentTime
        this.diffEnd = this.clipEndHandle - currentTime
        this.moveStart = true
        this.moveEnd = true
    }

    endMove() {
        this.moveStart = false
        this.moveEnd = false
    }

    clearMouseMoveHandlers() {
        this.seeking = false
        this.moveStart = false
        this.moveEnd = false
    }

    changeCurrent(e: MouseEvent) {
        let tm = this.computeTimeFromMouseEvent(e)
        this.sendAnalyticsEvent(this.AnalyticsCategory.MatchInfo, this.AnalyticsAction.GoToTimeline, '', tm)
        this.$emit('go-to-timestamp', tm)
    }

    computeTimeFromMouseEvent(e: MouseEvent): number {
        let bounds = this.$refs.parent.getBoundingClientRect()
        let percentage = (e.clientX - bounds.left) / (bounds.width)
        return this.start + (this.end - this.start) * percentage
    }

    recomputeSeekingTimeFromMouseEvent(e: MouseEvent) {
        if (this.disableSeeking) {
            return
        }
        
        let bounds = this.$refs.parent.getBoundingClientRect()
        this.seekingTime = this.computeTimeFromMouseEvent(e)

        Vue.nextTick(() => {
            let innerBounds = this.$refs.inner.getBoundingClientRect()
            let tooltipBounds = this.$refs.seekTick.getBoundingClientRect()
            this.tooltipX = innerBounds.right - innerBounds.left

            let newL = this.tooltipX - tooltipBounds.width / 2
            let newR = this.tooltipX + tooltipBounds.width / 2
            const rightBound = bounds.right - bounds.left
            if (newR > rightBound) {
                this.tooltipX = rightBound - tooltipBounds.width / 2
            } else if (newL < 0) {
                this.tooltipX = tooltipBounds.width / 2
            }

            this.tooltipY = -tooltipBounds.height - 8
        })
    }

    startSeeking(e : MouseEvent) {
        if (this.disableSeeking) {
            return
        }
        this.seeking = true
        this.recomputeSeekingTimeFromMouseEvent(e)
    }

    onMouseMove(e: MouseEvent) {
        let now = new Date()
        if ((now.getTime() - this.lastMouseMove.getTime()) < 33) {
            return
        }
        this.lastMouseMove = now

        let currentTime = this.computeTimeFromMouseEvent(e)
        let moveBoth = this.moveStart && this.moveEnd
        
        let newStart: number
        let newEnd: number

        if (moveBoth) {
            newStart = currentTime + this.diffStart
            newEnd = currentTime + this.diffEnd
        } else {
            newStart = currentTime
            newEnd = currentTime
        }

        if (this.moveStart) {
            this.$emit('update:clipStartHandle', newStart)
        }

        if (this.moveEnd) {
            this.$emit('update:clipEndHandle', newEnd)
        }

        if (this.seeking && !this.moveStart && !this.moveEnd) {
            this.recomputeSeekingTimeFromMouseEvent(e)
        }
    }

    get majorTicks(): number[] {
        return generateSteppedArrayRange(this.start, this.end, this.majorTickEvery)
    }

    get minorTicks(): number [] {
        let ticks: number[] = []
        let minorTickEvery = this.majorTickEvery / (this.intervalTicks + 1)

        this.majorTicks.forEach((t: number, idx: number) => {
            let start = t
            let end = (idx == this.majorTicks.length - 1) ? this.end : this.majorTicks[idx+1]
            let interval = generateSteppedArrayRange(start, end, minorTickEvery).slice(1)
            ticks.push(...interval)
        })
        return ticks
    }

    get style(): any {
        if (!this.fill) {
            return {
                'height': `${this.height}px`
            }
        } else {
            return {
                'height': '100%'
            }
        }
    }

    get computePercentage() : (t: number) => number {
        let start = this.start
        let end = this.end

        return (t: number) => {
            return (t - start) / (end - start) * 100
        }
    }

    get percentage(): number {
        let showTime = this.seeking ? this.seekingTime : this.current
        return this.computePercentage(showTime)
    }

    get progressBarStyle(): any {
        let style: any = {
            'width': `${this.percentage}%`
        }

        if (this.hasClipHandles) {
            style['background-color'] = 'transparent !important'
        }

        return style
    }

    tickDivStyle(t: number): any {
        return {
            'left': `${this.computePercentage(t)}%`
        }
    }

    eventTickMark(e: GenericEvent) : any {
        return {
            'border-right': `1px solid ${colorToCssString(e.color)}`
        }
    }

    get seekDivStyle(): any {
        return {
            'top': `${this.tooltipY}px`,
            'left': `${this.tooltipX}px`,
        }
    }

    get parentTickDivStyle(): any {
        return {
            'width': `calc(100%)`
        }
    }
}

</script>

<style scoped>

.parent-div {
    border-radius: 10px;
    border: 1px solid gray;
    background-color: #1E1E1E;
    z-index: 0;
    position: relative;
    user-select: none;
}

.bar-div {
    height: 100%;
    background-color: rgba(255, 255, 255, 0.1);
    border-right: 3px solid rgb(255, 215, 0);
    z-index: 1;
}

.clip-div {
    position: absolute;
    top: 0;
    width: 200px;
    height: 100%;
    background-color: rgba(255, 255, 255, 0.1);
    z-index: 5;
}

.clip-start-div{
    background-color: #4CAF50;
    height: 100%;
    width: 8px;
    cursor: pointer;
    z-index: 5;
    position: absolute;
    left: -5px;
    top: 0px;
}

.clip-end-div{
    background-color: #FF5252;
    height: 100%;
    width: 8px;
    cursor: pointer;
    z-index: 5;
    position: absolute;
    left: 100%;
    top: 0px;
}

.tick-parent {
    position: absolute;
    top: 0;
    overflow: hidden;
}

.event-div {
    z-index: 3 !important;
}

.tick {
    height: 100%;
    width: 1px;
}

.inner-tick {
    height: 100%;
}

.tick-div {
    position: absolute;
    bottom: 0;
    transform: translateX(-50%);
    z-index: 2;
}

.seek-div {
    position: absolute;
    padding: 5px 16px;
    background-color: rgba(97, 97, 97, 0.9);
    border-radius: 3px;
    transform: translateX(-50%);
}

.tick-mark {
    border-right: 1px solid white;
}

.major-tick {
    height: 95%;
}

.minor-tick {
    height: 20%;
}

.event-tick {
    height: 60%;
}

</style>