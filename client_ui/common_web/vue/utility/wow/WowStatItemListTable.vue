<template>
    <div>
        <div class="font-weight-bold text-h4 mb-4">
            {{ label }}
        </div>
        <v-divider></v-divider>
        <v-data-table
            :headers="statHeaders"
            :items="sortedItems"
            :items-per-page="sortedItems.length"
            hide-default-footer
        >
            <template v-slot:item.name="{ item }">
                <wow-character-display
                    class="px-0"
                    :character="item.character"
                    :patch="patch"
                >
                </wow-character-display>
            </template>

            <template v-slot:item.graph="{ item }">
                <div class="graph" :style="graphStyle(item)">
                </div>
            </template>

            <template v-slot:item.value="{ item }">
                {{ shortenNumberString(item.stat.value) }}
            </template>
        </v-data-table>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { WowStatItem } from '@client/js/wow/stats'
import { WowCharacter } from '@client/js/wow/character'
import { shortenNumberString } from '@client/js/strings'
import WowCharacterDisplay from '@client/vue/utility/wow/WowCharacterDisplay.vue'
import { specIdToColor } from '@client/js/wow/colors'

interface WowCharacterStatDisplay {
    character: WowCharacter
    stat: WowStatItem
    name: string
    value: number
}

@Component({
    components: {
        WowCharacterDisplay
    }
})
export default class WowStatItemListTable extends Vue {
    shortenNumberString = shortenNumberString

    @Prop({required: true})
    data!: WowStatItem[]

    @Prop({required: true})
    matchCharacters!: WowCharacter[]

    @Prop({default: 0})
    friendlyTeam!: number

    @Prop({required: true})
    patch!: string

    @Prop({default: 'Stats'})
    label!: string

    graphStyle(item: WowCharacterStatDisplay): any {
        return {
            'width': `${(item.value / this.maxValue) * 100}%`,
            'background-color': `var(--${specIdToColor(item.character.specId)})`,
        }
    }

    get guidToValue(): Map<string, number> {
        let ret = new Map()
        for (let d of this.data) {
            ret.set(d.guid, d.value)
        }
        return ret
    }

    get numTeams(): number {
        let set = new Set()
        for (let d of this.matchCharacters) {
            set.add(d.team)
        }
        return set.size
    }

    get maxValue(): number {
        return  Math.max(...this.data.map((ele: WowStatItem) => ele.value))
    }

    get sortedItems(): WowCharacterStatDisplay[] {
        return this.matchCharacters.map((ele: WowCharacter) => {
            let val = this.guidToValue.has(ele.guid) ? this.guidToValue.get(ele.guid)! : 0
            return {
                character: ele,
                stat: {
                    guid: ele.guid,
                    value: val,
                },
                name: ele.name,
                value: val,
            }
        }).sort((a: WowCharacterStatDisplay, b: WowCharacterStatDisplay) => {
            return b.stat.value - a.stat.value
        })
    }

    get statHeaders() : any[] {
        return [
            {
                text: 'Name',
                value: 'name',
                width: 200,
            },
            {
                text: 'Graph',
                value: 'graph',
                sortable: false,
                filterable: false,
            },
            {
                text: 'Value',
                value: 'value',
                width: 60,
            },
        ]
    }
}

</script>

<style scoped>

.graph {
    height: 32px;
}

</style>