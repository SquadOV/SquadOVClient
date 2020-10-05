<template>
    <div class="d-flex align-center">
        <v-tooltip v-if="!!value && !isValorantAccountValid" bottom offset-y>
            <template v-slot:activator="{on, attrs}">
                <v-icon v-bind="attrs" v-on="on" color="warning">
                    mdi-alert
                </v-icon>
            </template>

            <span>
                This Valorant account is configured incorrectly.
                Please click the edit button to add a login and password so we can properly pull your match history.
            </span>
        </v-tooltip>

        <v-select
            :value="value"
            @input="$emit('input', arguments[0])"
            label="Valorant Account"
            :items="items"
            solo
            hide-details
        >
        </v-select>

        <v-dialog
            v-if="!!value"
            v-model="showHideEdit"
            persistent
            max-width="40%"
        >
            <template v-slot:activator="{ on, attrs }">
                <v-btn
                    v-bind="attrs"
                    v-on="on"
                    icon
                    color="warning"
                >
                    <v-icon>
                        mdi-pencil
                    </v-icon>
                </v-btn>
            </template>

            <v-card>
                <v-card-title>
                    Edit Valorant Account
                </v-card-title>
                <v-divider></v-divider>

                <valorant-account-creation
                    ref="editForm"
                    :username="value.login"
                    @cancel="showHideEdit = false"
                    @save="onEditValorantAccount"
                >
                </valorant-account-creation>
            </v-card>
        </v-dialog>

        <v-dialog
            v-model="showHideNew"
            persistent
            max-width="40%"
        >
            <template v-slot:activator="{ on, attrs }">
                <v-btn
                    v-bind="attrs"
                    v-on="on"
                    icon
                    color="success"
                >
                    <v-icon>
                        mdi-plus
                    </v-icon>
                </v-btn>
            </template>

            <v-card>
                <v-card-title>
                    New Valorant Account
                </v-card-title>
                <v-divider></v-divider>

                <valorant-account-creation
                    ref="newForm"
                    @cancel="showHideNew = false"
                    @save="onSaveValorantAccount"
                >
                </valorant-account-creation>
            </v-card>            
        </v-dialog>

        <v-snackbar
            v-model="showHideAccountError"
            :timeout="5000"
            color="error"
        >
            You provided incorrect credentials.
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { ValorantAccountData } from '@client/js/valorant/valorant_account'
import { apiClient, ApiData } from '@client/js/api'

import ValorantAccountCreation from '@client/vue/utility/valorant/ValorantAccountCreation.vue'

@Component({
    components: {
        ValorantAccountCreation
    }
})
export default class ValorantAccountChooser extends Vue {
    $refs!: {
        editForm: ValorantAccountCreation
        newForm: ValorantAccountCreation
    }

    @Prop({required: true})
    value! : ValorantAccountData | null

    @Prop({type: Array, required: true})
    options! : ValorantAccountData[]

    showHideEdit : boolean = false
    showHideNew : boolean = false
    showHideAccountError : boolean = false

    get isValorantAccountValid() : boolean {
        return !!this.value?.login && !!this.value?.encryptedPassword
    }

    get items() : any[] {
        return this.options!.map((ele : ValorantAccountData) => ({
            text: `${ele.username}#${ele.tag}`,
            value: ele,
        }))
    }

    onEditValorantAccount(username : string, password : string) {
        if (!this.value) {
            return
        }

        apiClient.editValorantAccount(this.value.puuid, username, password).then((resp : ApiData<ValorantAccountData>) => {
            Vue.set(this.value!, 'login', resp.data.login)
            Vue.set(this.value!, 'encryptedPassword', resp.data.encryptedPassword)
            this.showHideEdit = false
        }).catch((err : any ) => {
            this.showHideAccountError = true
            console.log('Failed to edit valorant account: ', err)
        }).finally(() => {
            this.$refs.editForm.clear()
        })
    }

    onSaveValorantAccount(username : string, password : string) {
        apiClient.newValorantAccount(username, password).then((resp : ApiData<ValorantAccountData>) => {
            this.$emit('update:options', [...this.options, resp.data])
            this.$emit('input', resp.data)
            this.showHideNew = false
        }).catch((err : any ) => {
            this.showHideAccountError = true
            console.log('Failed to create valorant account: ', err)
        }).finally(() => {
            this.$refs.newForm.clear()
        })
    }
}

</script>

<style scoped>

</style>