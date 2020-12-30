<template>
    <div>
        <v-tabs>
            <v-tab
                v-for="(d, idx) in data"
                :key="`tab-${idx}`"
            >
                {{ d.title }}
            </v-tab>
        </v-tabs>

        
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import {
    UnifiedWowEventContainer,
    SerializedWowMatchEvents
} from '@client/js/wow/events'

@Component
export default class WowTimeline extends Vue {
    @Prop({required: true})    
    startTime!: Date

    @Prop({required: true})    
    endTime!: Date

    @Prop({required: true})
    events!: SerializedWowMatchEvents | null

    @Prop({required: true})
    unifiedEvents!: UnifiedWowEventContainer[]

    data: any[] = [
        {
            title: 'Damage Done'
        },
        {
            title: 'Heals'
        },
        {
            title: 'Damage Received'
        },
    ]
}

</script>