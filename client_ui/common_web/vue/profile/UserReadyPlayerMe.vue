<template>
    <v-dialog v-model="showHideAvatar" persistent max-width="80%">
        <template v-slot:activator="{ on, attrs }">
            <div class="d-flex justify-center">
                <v-btn
                    v-bind="attrs"
                    v-on="on"
                    color="primary"
                    @click="triggerListener"
                >
                    <v-avatar 
                        v-if="!avatarLoading"
                        color="primary" 
                        size="32px"
                        class="rpmLogo"
                    >
                        <v-img
                            width="32px"
                            max-width="32px"
                            contain
                            :src="$root.generateAssetUri('assets/logos/readyplayerme.png')"
                        >
                        </v-img>
                    </v-avatar>
                    <v-progress-circular
                        v-if="avatarLoading"
                        indeterminate
                        color="white"
                        :size="32"
                        class="rpmLogo"
                    ></v-progress-circular>
                    Create Avatar
                </v-btn>
            </div>
        </template>
        <v-card>
            <v-card-title>
                Create Your Avatar
            </v-card-title>
            <v-divider></v-divider>
            <div>
                <iframe
                    src="https://squadov.readyplayer.me/"
                    width="100%"
                    height="720px"
                    frameborder="0"
                    allow="camera *; microphone *"
                >
                </iframe>
            </div>
            <v-card-actions>
                <v-btn color="error" @click="cancelAvatarCreation">
                    Cancel
                </v-btn>
                <v-spacer></v-spacer>
                <v-btn color="success" @click="saveAvatarCreation" :disabled="!isUrlGenerated">
                    Save
                </v-btn>
            </v-card-actions>
        </v-card>
    </v-dialog>
</template>

<script lang="ts">
import Vue from "vue"
import Component from "vue-class-component"
import LoadingContainer from "@client/vue/utility/LoadingContainer.vue"
import { createReadyPlayerMeRender } from "@client/js/readyPlayerMe"
import axios from "axios"

@Component({
    components: {
        LoadingContainer,
    },
})
export default class UserReadyPlayerMe extends Vue {
    avatarLoading: boolean = false
    showHideAvatar: boolean = false
    avatarURL: string = ""
    isUrlGenerated: boolean = false
    readyPlayerMeFile: File | null = null

    passRPMFile() {
        this.$emit('input', this.readyPlayerMeFile)
    }

    triggerListener() {
        window.addEventListener("message", this.receiveMessage, false)
    }

    receiveMessage(event: MessageEvent) {
        window.removeEventListener("message", this.receiveMessage, false)
        this.avatarURL = event.data
        this.isUrlGenerated = true
    }

    generateImage(url: string) {
        createReadyPlayerMeRender(url)
            .then((response) => {
                this.onReadyPlayerMeUrlUpdate(response)
            })
            .catch((err) => console.log(err))
            .finally(() => this.avatarLoading = false)
    }

    cancelAvatarCreation() {
        this.showHideAvatar = false
        this.avatarLoading = false
    }

    saveAvatarCreation() {
        this.showHideAvatar = false
        this.generateImage(this.avatarURL)
        this.avatarLoading = true
        this.isUrlGenerated = false
    }
    
    async onReadyPlayerMeUrlUpdate (urlPath: string) {
        return axios.get(urlPath, { 
            headers: { "Content-Type": "multipart/form-data" },
            responseType: 'blob'
        })
        .then((response) => {
            const blob = response.data
            const file = new File([blob], 'readyPlayerMe.png')
            this.readyPlayerMeFile = file
        })
        .finally(() => this.passRPMFile())
    }
}
</script>

<style>
.rpmLogo {
    right: 10px
}
.fixedspinner {
    position: fixed;
}
</style>