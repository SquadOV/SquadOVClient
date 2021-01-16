<template>
    <v-data-table
        :headers="headers"
        :items="items"
        hide-default-footer
    >
        <template v-slot:item.user="{ item }">
            <div class="d-flex align-center">
                <div class="d-flex justify-center text-h6" :style="placementStyle(item.user.placement)">{{ getOrdinal(item.user.placement) }}</div>

                <tft-little-legend-icon
                    class="mx-4"
                    :content-id="item.user.companion.contentId"
                    :width="36"
                    :height="36"
                >
                </tft-little-legend-icon>

                <div>
                    {{ match.puuidToName[item.user.puuid] }}
                </div>
            </div>            
        </template>

        <template v-slot:item.traits="{ item }">
            <tft-full-trait-display
                class="mx-2 my-1"
                :traits="item.traits"
                :tft-set="patch"
                :single-width-height="24"
            >
            </tft-full-trait-display>
        </template>

        <template v-slot:item.units="{ item }">
            <tft-full-unit-display
                class="mx-2 my-1"
                :units="item.units"
                :tft-set="patch"
                :single-width="36"
                :margin="3"
            >
            </tft-full-unit-display>
        </template>
    </v-data-table>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { WrappedTftMatch, TftParticipant } from '@client/js/tft/matches'
import { getOrdinal } from '@client/js/ordinal'
import {
    colorToCssString,
    getGenericFirstPlaceColor,
    getGenericTopPlaceColor,
    getGenericBottomPlaceColor
} from '@client/js/color'
import TftFullTraitDisplay from '@client/vue/utility/tft/TftFullTraitDisplay.vue'
import TftFullUnitDisplay from '@client/vue/utility/tft/TftFullUnitDisplay.vue'
import TftLittleLegendIcon from '@client/vue/utility/tft/TftLittleLegendIcon.vue'

@Component({
    components: {
        TftFullTraitDisplay,
        TftFullUnitDisplay,
        TftLittleLegendIcon,
    }
})
export default class TftScoreboard extends Vue {
    getOrdinal: any = getOrdinal

    @Prop({required: true})
    match!: WrappedTftMatch

    @Prop({required: true})
    patch!: string

    placementStyle(place: number): any {
        return {
            'color': colorToCssString((place > 4) ? getGenericBottomPlaceColor() :
                (place > 1) ? getGenericTopPlaceColor() :
                getGenericFirstPlaceColor())
        }
    }

    get headers(): any[] {
        return [
            {
                text: 'Player',
                value: 'user',
                sortable: false,
                filterable: false,
            },
            {
                text: 'Traits',
                value: 'traits',
                sortable: false,
                filterable: false,
                width: 150,
                align: 'center',
            },
            {
                text: 'Units',
                value: 'units',
                sortable: false,
                filterable: false,
                align: 'center',
            },
            {
                text: 'Last Round',
                value: 'lastRound',
                width: 40,
                sortable: false,
                filterable: false,
                align: 'center',
            },
            {
                text: 'Gold Left',
                value: 'goldLeft',
                width: 40,
                sortable: false,
                filterable: false,
                align: 'center',
            },
            {
                text: 'Players Eliminated',
                value: 'playersEliminated',
                width: 40,
                sortable: false,
                filterable: false,
                align: 'center',
            },
            {
                text: 'Player Damage',
                value: 'totalDamageToPlayers',
                width: 40,
                sortable: false,
                filterable: false,
                align: 'center',
            },
        ]
    }

    get items(): any[] {
        let participants = [...this.match.data.info.participants]
        participants.sort((a: TftParticipant, b: TftParticipant) => {
            return a.placement - b.placement
        })

        return participants.map((ele: TftParticipant) => {
            return {
                user: ele,
                traits: ele.traits,
                units: ele.units,
                lastRound: ele.lastRound,
                goldLeft: ele.goldLeft,
                playersEliminated: ele.playersEliminated,
                totalDamageToPlayers: ele.totalDamageToPlayers,
                value: ele,
            }
        })
    }
}

</script>