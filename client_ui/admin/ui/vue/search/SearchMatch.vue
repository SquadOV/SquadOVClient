<template>
    <v-tabs>
        <v-tab>
            Aim Lab
        </v-tab>

        <v-tab-item>
            <aimlab-search></aimlab-search>
        </v-tab-item>

        <v-tab>
            CS:GO
        </v-tab>

        <v-tab-item>
            <csgo-search></csgo-search>
        </v-tab-item>

        <v-tab>
            Hearthstone
        </v-tab>

        <v-tab-item>
            <hearthstone-search></hearthstone-search>
        </v-tab-item>

        <v-tab>
            LoL
        </v-tab>

        <v-tab-item>
            <lol-search></lol-search>
        </v-tab-item>

        <v-tab>
            TFT
        </v-tab>

        <v-tab-item>
            <tft-search></tft-search>
        </v-tab-item>

        <v-tab>
            Valorant
        </v-tab>

        <v-tab-item>
            <valorant-search></valorant-search>
        </v-tab-item>

        <v-tab>
            WoW
        </v-tab>

        <v-tab-item>
            <wow-search></wow-search>
        </v-tab-item>
    </v-tabs>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'

import AimlabSearch from '@client/vue/search/games/AimlabSearch.vue'
import CsgoSearch from '@client/vue/search/games/CsgoSearch.vue'
import HearthstoneSearch from '@client/vue/search/games/HearthstoneSearch.vue'
import LolSearch from '@client/vue/search/games/LolSearch.vue'
import TftSearch from '@client/vue/search/games/TftSearch.vue'
import ValorantSearch from '@client/vue/search/games/ValorantSearch.vue'
import WowSearch from '@client/vue/search/games/WowSearch.vue'

@Component({
    components: {
        AimlabSearch,
        CsgoSearch,
        HearthstoneSearch,
        LolSearch,
        TftSearch,
        ValorantSearch,
        WowSearch,
    }
})
export default class SearchMatch extends Vue {
    
}

</script>
