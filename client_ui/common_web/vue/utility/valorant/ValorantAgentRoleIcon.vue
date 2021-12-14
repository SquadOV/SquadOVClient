<template>
    <div>
        <v-tooltip
            offset-y bottom
            open-delay="3"
        >
            <template v-slot:activator="{on, attrs}">
                <v-img
                    v-bind="attrs"
                    v-on="on"
                    :src="$root.generateAssetUri(iconFname)"
                    :max-width="widthHeight"
                    :max-height="widthHeight"
                    contain
                >
                </v-img>
            </template>

            {{ roleName }}
        </v-tooltip>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { ValorantAgentRole, valorantRoleToName } from '@client/js/valorant/valorant_utility'

@Component
export default class ValorantAgentRoleIcon extends Vue {
    @Prop({required: true})
    role!: ValorantAgentRole

    @Prop({type: Number, default: 50})
    widthHeight!: number

    get iconFname(): string {
        let base = 'unknown'
        switch (this.role) {
            case ValorantAgentRole.Controller:
                base = 'controller'
                break
            case ValorantAgentRole.Duelist:
                base = 'duelist'
                break
            case ValorantAgentRole.Initiator:
                base = 'initiator'
                break
            case ValorantAgentRole.Sentinel:
                base = 'sentinel'
                break
        }
        return `assets/valorant/roles/${base}.png`
    }

    get roleName(): string {
        return valorantRoleToName(this.role)
    }
}

</script>