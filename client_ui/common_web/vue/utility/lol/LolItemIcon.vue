<template>
    <div>
        <template v-if="itemId > 0">
            <v-tooltip bottom>
                <template v-slot:activator="{on, attrs}">
                    <v-img
                        :src="src"
                        :width="widthHeight"
                        :height="widthHeight"
                        :max-width="widthHeight"
                        :max-height="widthHeight"
                        contain
                        v-on="on"
                        v-bind="attrs"
                    >
                    </v-img>
                </template>

                {{ itemName }}
            </v-tooltip>
        </template>

        <template v-else>
            <div :style="emptyItemStyle">
            </div>
        </template>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { LolParticipantStats } from '@client/js/lol/participant'
import { ddragonContainer } from '@client/js/lolDdragon'

@Component
export default class LolItemIcon extends Vue {
    @Prop({required: true})
    itemId!: number

    @Prop({default: 32})
    widthHeight!: number

    @Prop({required: true})
    gameVersion!: string

    itemName: string = ''

    get src(): string {
        return ddragonContainer.getClientForVersion(this.gameVersion).getLolItemIconUrl(this.itemId)
    }

    @Watch('itemId')
    @Watch('gameVersion')
    refreshData() {
        if (this.itemId === 0) {
            this.itemName = 'None'
            return
        }

        this.itemName = 'Loading...'
        ddragonContainer.getClientForVersion(this.gameVersion).getLolItemName(this.itemId).then((resp: string) => {
            this.itemName = resp
        }).catch((err: any) => {
            console.log('Failed to get item name: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }

    get emptyItemStyle() : any {
        return {
            width: `${this.widthHeight}px`,
            height: `${this.widthHeight}px`,
            'max-width': `${this.widthHeight}px`,
            'max-height': `${this.widthHeight}px`,
        }
    }
}

</script>