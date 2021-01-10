<template>
    <v-row>
        <v-col cols="4">
            <svg width="100%" height="90px" viewBox="0 0 40 80">
                <path id="Head"
                        :fill="headFill" stroke="black" stroke-width="0.2"
                        d="M 16.74,1.99
                        C 24.76,-1.48 26.62,6.26 22.69,9.38
                            18.80,12.46 12.82,7.64 16.74,1.99 Z" />
                <path id="Torso"
                        :fill="bodyFill" stroke="black" stroke-width="0.2"
                        d="M 17.00,11.00
                        C 17.00,11.00 23.00,11.00 23.00,11.00
                            23.00,11.00 30.01,17.33 30.01,17.33
                            33.07,21.41 35.50,32.78 35.66,38.00
                            35.71,39.87 36.13,43.79 34.99,45.26
                            33.98,46.58 32.46,46.66 31.00,47.00
                            30.99,39.17 30.77,29.92 27.00,23.00
                            26.74,27.03 27.45,31.26 25.55,35.00
                            21.60,42.74 15.87,40.14 13.65,33.00
                            12.78,30.20 13.20,26.10 13.00,23.00
                            9.23,29.92 9.01,39.17 9.00,47.00
                            7.54,46.66 6.02,46.58 5.01,45.26
                            3.87,43.79 4.29,39.87 4.34,38.00
                            4.50,32.78 6.93,21.41 9.99,17.33
                                9.99,17.33 17.00,11.00 17.00,11.00 Z" />
                <path id="Legs"
                        :fill="legFill" stroke="black" stroke-width="0.2"
                        d="M 12.36,35.00
                        C 12.36,35.00 19.64,41.82 19.64,41.82
                            19.64,41.82 22.73,39.18 22.73,39.18
                            22.73,39.18 26.00,35.27 26.00,35.27
                            26.00,35.27 28.27,36.09 28.27,36.09
                            28.82,44.00 28.08,48.34 28.27,55.55
                            28.71,58.99 28.33,62.84 29.00,66.09
                            29.00,66.09 30.87,80.00 30.87,80.00
                            30.87,80.00 26.64,79.91 26.64,79.91
                            26.64,79.91 24.91,69.91 24.91,69.91
                            24.91,69.91 21.27,46.00 21.27,46.00
                            21.27,46.00 18.45,46.18 18.45,46.18
                            18.45,46.18 15.09,69.73 15.09,69.73
                            15.09,69.73 13.09,79.82 13.09,79.82
                            13.09,79.82 9.13,80.00 9.13,80.00
                            9.13,80.00 10.55,67.18 10.55,67.18
                            11.22,63.93 11.33,59.44 11.76,56.00
                            11.95,48.79 11.64,44.09 12.36,35.00 Z
                        M 48.36,68.91" />
            </svg>
        </v-col>

        <v-col cols="8" class="d-flex flex-column justify-space-around">
            <div>
                Head: <span class="percentage" :style="{ color: headFill }">{{ hspStr }}</span> ({{ headshots }})
            </div>
            
            <div>
                Body: <span class="percentage" :style="{ color: bodyFill }">{{ bspStr }}</span> ({{ bodyshots }})
            </div>

            <div>
                Legs: <span class="percentage" :style="{ color: legFill }">{{ lspStr }}</span> ({{ legshots }})
            </div>
        </v-col>
    </v-row>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'

@Component
export default class ValorantHitTracker extends Vue {
    @Prop({type : Number, default: 0})
    headshots! : number

    @Prop({type : Number, default: 0})
    bodyshots! : number

    @Prop({type : Number, default: 0})
    legshots! : number

    percentageToOpacity(p : number) : number {
        const minOpacity = 0.2
        const maxOpacity = 1.0
        return Math.min(Math.max(minOpacity + (maxOpacity - minOpacity) * p, minOpacity), maxOpacity)
    }

    get totalShots() : number {
        return this.headshots + this.bodyshots + this.legshots
    }

    get hsp() : number {
        if (this.totalShots == 0) {
            return 0;
        }
        return this.headshots / this.totalShots
    }

    get hspStr() : string {
        return `${(this.hsp * 100.0).toFixed(2)}%`
    }

    get headFill() : string {
        return `rgba(25, 118, 210, ${this.percentageToOpacity(this.hsp)})`
    }

    get bsp() : number {
        if (this.totalShots == 0) {
            return 0;
        }
        return this.bodyshots / this.totalShots
    }

    get bspStr() : string {
        return `${(this.bsp * 100.0).toFixed(2)}%`
    }

    get bodyFill() : string {
        return `rgba(25, 118, 210, ${this.percentageToOpacity(this.bsp)})`
    }

    get lsp() : number {
        if (this.totalShots == 0) {
            return 0;
        }
        return this.legshots / this.totalShots
    }

    get lspStr() : string {
        return `${(this.lsp * 100.0).toFixed(2)}%`
    }

    get legFill() : string {
        return `rgba(25, 118, 210, ${this.percentageToOpacity(this.lsp)})`
    }
}

</script>

<style scoped>

.percentage {
    font-weight: 700;
}

</style>