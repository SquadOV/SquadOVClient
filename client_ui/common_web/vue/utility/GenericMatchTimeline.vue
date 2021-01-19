<template>
   <div
        class="parent-div"
        :style="style"
    >
        <div class="bar-div" :style="progressBarStyle">
        </div>

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
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { GenericEvent} from '@client/js/event'
import { generateSteppedArrayRange } from '@client/js/array'
import { colorToCssString } from '@client/js/color'

@Component
export default class GenericMatchTimeline extends Vue {
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

    @Prop({default: 0})
    current!: number

    @Prop({type: Array, required: true})
    inputEvents!: GenericEvent[]

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
        return {
            'height': `${this.height}px`
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
        return this.computePercentage(this.current)
    }

    get progressBarStyle(): any {
        return {
            'width': `${this.percentage}%`
        }
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
}

</script>

<style scoped>

.parent-div {
    border-radius: 10px;
    border: 1px solid gray;
    background-color: #1E1E1E;
    z-index: 0;
    position: relative;
    overflow: hidden;
}

.bar-div {
    height: 100%;
    background-color: rgba(255, 255, 255, 0.1);
    border-right: 3px solid rgb(255, 215, 0);
    z-index: 1;
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