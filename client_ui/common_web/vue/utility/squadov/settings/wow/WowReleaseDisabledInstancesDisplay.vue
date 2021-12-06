<template>
    <loading-container :is-loading="choices === null">
        <template v-slot:default="{ loading }">
            <div class="d-flex align-center">
                <v-autocomplete
                    class="my-4"
                    :value="valueIds"
                    :items="items"
                    @input="onChangeIds"
                    outlined
                    chips
                    small-chips
                    multiple
                    prepend-icon="mdi-magnify"
                    hide-details
                    :loading="loading"
                >
                </v-autocomplete>

                <v-btn
                    class="ml-2"
                    color="accent"
                    @click="selectAll"
                    small
                >
                    All
                </v-btn>

                <v-btn
                    class="ml-2"
                    color="accent"
                    @click="selectNone"
                    small
                >
                    None
                </v-btn>
            </div>

            <div
                class="instance-list"
                v-if="!loading"
            >
                <v-list-item-group multiple :value="valueIds" @change="onChangeIds">
                    <v-list-item
                        v-for="(item, idx) in finalChoices"
                        :value="item.id"
                        :key="`item-${idx}`"
                    >
                        <template v-slot="{active}">
                            <v-list-item-action>
                                <v-checkbox :input-value="active"></v-checkbox>
                            </v-list-item-action>

                            <v-list-item-content>
                                {{ item.name }}
                            </v-list-item-content>
                        </template>
                    </v-list-item>
                </v-list-item-group>
            </div>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { WowDisabledInstance } from '@client/js/system/settings'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { SquadOvWowRelease } from '@client/js/squadov/game'
import { staticClient } from '@client/js/staticData'
import { ApiData } from '@client/js/api'
import { WowContentDatum } from '@client/js/wow/staticCache'
import axios from 'axios'

@Component({
    components: {
        LoadingContainer,
    }
})
export default class WowReleaseDisabledInstancesDisplay extends Vue {
    @Prop({required: true})
    release!: SquadOvWowRelease

    @Prop({required: true})
    value!: WowDisabledInstance[]

    choices: WowContentDatum[] | null = null

    selectAll() {
        this.onChangeIds(this.finalChoices.map((ele: WowContentDatum) => ele.id))
    }

    selectNone() {
        this.onChangeIds([])
    }

    get valueIds(): number[] {
        return this.value.map((ele: WowDisabledInstance) => ele.id)
    }

    onChangeIds(values: number[]) {
        this.$emit('input', values.map((id: number) => {
            return {
                id,
                release: this.release,
            }
        }))
    }

    get items(): any[] {
        return this.finalChoices.map((ele: WowContentDatum) => {
            return {
                text: ele.name,
                value: ele.id,
            }
        })
    }

    get finalChoices(): WowContentDatum[] {
        if (!this.choices) {
            return []
        }
        return this.choices.sort((a: WowContentDatum, b: WowContentDatum) => {
            if (a.name < b.name) {
                return -1
            } else if (a.name > b.name) {
                return 1
            } else {
                return 0
            }
        })
    }

    @Watch('release')
    refreshChoices() {
        this.choices = null
        axios.get(staticClient.getWowContentInstancesUrl(this.release)).then((resp: ApiData<WowContentDatum[]>) => {
            this.choices = resp.data
        }).catch((err: any) => {
            console.error('Failed to get wow content instances: ', err)
        })
    }

    mounted() {
        this.refreshChoices()
    }
}

</script>

<style scoped>

>>>.instance-list {
    overflow-y: auto;
    max-height: 650px;
}

</style>