<template>
    <loading-container :is-loading="!allCharacters">
        <template v-slot:default="{ loading }">
            <div v-if="!loading">
                <v-row>
                    <v-col cols="6">
                        <div class="d-flex align-center">
                            <wow-character-chooser
                                v-model="selectedCharacter"
                                :characters="allCharacters"
                                class="mr-4"
                            >
                            </wow-character-chooser>

                            <v-btn icon @click="refreshData">
                                <v-icon>
                                    mdi-refresh
                                </v-icon>
                            </v-btn>
                        </div>
                    </v-col>
                </v-row>
                <router-view v-if="hasSelectedCharacter"></router-view>
                <v-row justify="center" align="center" v-else>
                    <v-col cols="8">
                        <h2 class="text-center">
                            <span v-if="isLocalUser">
                                Run SquadOV while running a dungeon or raid in WoW and SquadOV will automatically detect your characters.
                                Be sure to be running an auto-combat log enabler such as <a href="#" @click="gotoAutolog">AutoLog</a> or <a href="#" @click="gotoLoggerHead">LoggerHead</a>.
                                If you're doing arena runs, use <a href="#" @click="gotoArenaCombatLogger">Arena Combat Logger</a> instead.
                            </span>

                            <span v-else>
                                No WoW characters found for the selected user.
                            </span>
                        </h2>
                    </v-col>
                </v-row>
            </div>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { apiClient, ApiData } from '@client/js/api'
import { WowCharacter } from '@client/js/wow/character'
import WowCharacterChooser from '@client/vue/utility/wow/WowCharacterChooser.vue'

/// #if DESKTOP
import { shell } from 'electron'
/// #endif

const maxTasksPerRequest : number = 10

@Component({
    components: {
        LoadingContainer,
        WowCharacterChooser
    }
})
export default class WowLogContainer extends Vue {
    @Prop({required: true})
    userId!: number

    @Prop({required: true})
    guid!: string | undefined

    allCharacters : WowCharacter[] | null = null
    selectedCharacter : WowCharacter | null | undefined = null

    get isLocalUser(): boolean {
        return this.userId === this.$store.state.currentUser.id
    }

    get hasSelectedCharacter() : boolean {
        return !!this.selectedCharacter
    }

    gotoAutolog() {
        const url = 'https://www.curseforge.com/wow/addons/autolog'
/// #if DESKTOP
        shell.openExternal(url)
/// #else
        window.open(url, '_blank')
/// #endif
    }

    gotoLoggerHead() {
        const url = 'https://www.curseforge.com/wow/addons/loggerhead'
/// #if DESKTOP
        shell.openExternal(url)
/// #else
        window.open(url, '_blank')
/// #endif
    }

    gotoArenaCombatLogger() {
        const url = 'https://www.curseforge.com/wow/addons/arena-combat-logger'
/// #if DESKTOP
        shell.openExternal(url)
/// #else
        window.open(url, '_blank')
/// #endif
    }

    @Watch('selectedCharacter')
    onSelectCharacter() {
        if (!this.selectedCharacter) {
            return;
        }

        if (this.selectedCharacter.guid === this.guid) {
            return;
        }

        const params : any = {
            name: this.$route!.name!,
            params: {
                userId: this.userId,
                guid: this.selectedCharacter.guid
            },
            query: this.$route.query
        }

        if (!this.guid) {
            this.$router.replace(params)
        } else {
            this.$router.push(params)
        }
    }

    @Watch('userId')
    refreshData() {
        this.allCharacters = null
        apiClient.listWoWCharacters(this.userId).then((resp: ApiData<WowCharacter[]>) => {
            this.allCharacters = resp.data
            if (this.allCharacters!.length > 0) {
                if (!!this.guid) {
                    this.selectedCharacter = this.allCharacters!.find((ele: WowCharacter) => {
                        return ele.guid == this.guid
                    })
                } else {
                    this.selectedCharacter = this.allCharacters[0]
                }
            } else {
                this.selectedCharacter = null
            }
        })
    }


    mounted() {
        this.refreshData()
    }
}

</script>

<style scoped>

</style>