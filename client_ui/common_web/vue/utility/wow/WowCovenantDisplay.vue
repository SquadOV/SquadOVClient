<template>
    <div>
        <div class="d-flex align-center">
            <div class="text-h6 font-weight-bold mr-2">Covenant: </div>
            <div class="text-h6">{{ covenantName }}, {{ soulbindName }}</div>
        </div>

        <div class="d-flex align-center mt-2">
            <div class="text-subtitle-1 font-weight-bold ml-8 mr-2">Soulbind Traits: </div>
            <wow-talent-icon
                class="mr-1"
                v-for="(tid, idx) in covenant.soulbindTraits"
                :talent-id="tid"
                :key="`talent-${idx}`"
                hide-if-socket
            >
            </wow-talent-icon>
        </div>

        <div class="d-flex align-center mt-2">
            <div class="text-subtitle-1 font-weight-bold ml-8 mr-2">Conduits: </div>
            <wow-conduit-icon
                class="mr-1"
                v-for="(c, idx) in covenant.conduits"
                :conduit-id="c.itemId"
                :ilvl="c.ilvl"
                :key="`conduit-${idx}`"
            >
            </wow-conduit-icon>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { WowCovenant } from '@client/js/wow/character'
import { wowCache } from '@client/js/wow/staticCache'
import WowTalentIcon from '@client/vue/utility/wow/WowTalentIcon.vue'
import WowConduitIcon from '@client/vue/utility/wow/WowConduitIcon.vue'

@Component({
    components: {
        WowTalentIcon,
        WowConduitIcon
    }
})
export default class WowCovenantDisplay extends Vue {
    @Prop({required: true})
    covenant!: WowCovenant

    covenantName: string = ''
    soulbindName: string = ''

    @Watch('convenant')
    refreshCovenantData() {
        wowCache.getCovenant(this.covenant.covenantId).then((resp: string) => {
            this.covenantName = resp
        }).catch((err: any) => {
            console.log('Failed to get covenant: ', err)
        })

        wowCache.getSoulbind(this.covenant.soulbindId).then((resp: string) => {
            this.soulbindName = resp
        }).catch((err: any) => {
            console.log('Failed to get covenant: ', err)
        })
    }

    mounted() {
        this.refreshCovenantData()
    }
}

</script>