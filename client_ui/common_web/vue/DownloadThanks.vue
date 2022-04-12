<template>
    <div class="d-flex flex-column justify-center align-center" id="thank-container">
        <div class="font-weight-bold text-h4">
            Thanks for downloading SquadOV!
        </div>

        <div class="mt-1">
            If you're having trouble downloading the installer, try clicking <a :href="altDownloadLink" target="_blank">here</a>.
        </div>

        <div class="mt-1">
            Still having trouble? Check out our support article <a :href="supportLink" target="_blank">here</a>.
        </div>

        <template v-if="!isDesktop">
            <div class="mt-8">
                Did you know we have a Discord? Come join a growing community and get access to better support!
            </div>

            <v-btn
                class="mt-4"
                :href="discordUrl"
                target="_blank"
                color="primary"
            >
                Discord
            </v-btn>
        </template>
    </div>
</template>

<script lang="ts">

import CommonComponent from '@client/vue/CommonComponent'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { apiClient } from '@client/js/api'
import { AnalyticsAction, AnalyticsCategory } from '@client/js/analytics/events'

@Component
export default class DownloadThanks extends CommonComponent {
    @Prop({type: Boolean, default: false})
    wizard!: boolean

    get downloadLink(): string {
        return 'https://us-central1.content.squadov.gg/builds/SquadOV.exe'
    }

    get altDownloadLink(): string {
        return 'https://storage.googleapis.com/us-central1.content.squadov.gg/builds/SquadOV.exe'
    }

    get discordUrl(): string {
        return 'https://discord.gg/6Rj5jCVDeC'
    }

    get supportLink(): string {
        return 'https://squadov.zendesk.com/hc/en-us/articles/4497746872475-Error-Forbidden-or-Not-Found-when-downloading-SquadOV'
    }

    get isDesktop(): boolean {
/// #if DESKTOP
        return true
/// #else
        return false
/// #endif
    }

    mounted() {
        if (!this.isDesktop) {
            let link = document.createElement('a')
            link.href = this.downloadLink
            link.download = 'SquadOV.exe'
            link.click()

            if (!!this.$store.state.currentUser) {
                apiClient.markUserDownload().then(() => {
                }).catch((err: any) => {
                    console.error('Failed to mark user download: ', err)
                })
            }

            if (this.wizard) {
                this.analytics?.event(this.$route, AnalyticsCategory.SetupWizard, AnalyticsAction.SetupDoDownload, '', 0)
            }
        }
    }
}

</script>

<style scoped>

#thank-container {
    width: 100%;
    height: calc(100% - 48px - 8px);
}

</style>