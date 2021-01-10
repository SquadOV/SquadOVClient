<template>
    <div class="ma-4">
        <v-form>
            <v-text-field
                class="my-3"
                v-model="workingUsername"
                label="Username"
                outlined
                hide-details
            >
            </v-text-field>

            <v-text-field
                class="my-3"
                v-model="workingPassword"
                label="Password"
                outlined
                type="password"
                hide-details
            >
            </v-text-field>
        </v-form>

        <div class="d-flex">
            <v-btn
                :loading="inProgress"
                color="error"
                @click="$emit('cancel')"
            >
                Cancel
            </v-btn>
            
            <v-spacer></v-spacer>

            <v-btn
                :loading="inProgress"
                color="success"
                @click="onSave"
            >
                Save
            </v-btn>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'

@Component
export default class ValorantAccountCreation extends Vue {
    @Prop({default: ''})
    username! : string | null

    workingUsername: string = ''
    workingPassword: string = ''

    inProgress: boolean = false

    onSave() {
        this.inProgress = true
        this.$emit('save', this.workingUsername, this.workingPassword)
    }

    @Watch('username')
    syncUsername() {
        if (!!this.username) {
            this.workingUsername = this.username
        } else {
            this.workingUsername = ''
        }
    }

    clear() {
        this.syncUsername()
        this.workingPassword = ''
        this.inProgress = false
    }

    mounted() {
        this.clear()
    }
}

</script>