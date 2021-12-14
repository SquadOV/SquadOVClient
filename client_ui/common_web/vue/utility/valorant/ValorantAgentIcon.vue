<template>
    <div>
        <v-tooltip
            offset-y bottom
            open-delay="3"
        >
            <template v-slot:activator="{on, attrs}">
                <v-img
                    :src="$root.generateAssetUri(agentFname)"
                    :max-width="widthHeight"
                    :max-height="widthHeight"
                    :style="style"
                    contain
                >
                </v-img>
            </template>

            {{ agentFullName }}
        </v-tooltip>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { getValorantContent } from '@client/js/valorant/valorant_content'

@Component
export default class ValorantAgentIcon extends Vue {
    @Prop({required: true})
    agent! : string

    @Prop({type: Number, default: 50})
    widthHeight!: number

    @Prop({type: Boolean, default: false})
    circular!: boolean

    get agentName() : string {
        let cnt = getValorantContent(null)
        return cnt.agentIdToFilesystemName(this.agent)
    }

    get agentFname() : string {
        if (this.agent.length === 0) {
            return 'assets/wow/Unknown.png'
        } else {
            return `assets/valorant/agents/${this.agentName}.png`
        }
    }

    get agentFullName(): string {
        return getValorantContent(null).agentIdToName(this.agent)
    }

    get style() : any {
        if (!this.circular) {
            return {}
        }

        return {
            'border-radius': '50%',
            'border': '1px solid white',
        }
    }
}

</script>