<template>
    <div class="d-flex align-center">
        <div class="mr-1">
            {{ label }}
        </div>

        <div class="d-flex align-center">
            <valorant-agent-chooser
                class="member"
                v-for="(v, idx) in value"
                :value="v"
                @input="onChangeAgent(idx, arguments[0])"
                :key="idx"
            >
            </valorant-agent-chooser>
        </div>

        <v-btn
            color="error"
            icon
            :disabled="value.length === 0"
            @click="removeFromComp"
        >
            <v-icon> 
                mdi-minus
            </v-icon>
        </v-btn>

        <v-btn
            color="success"
            icon
            @click="addToComp"
        >
            <v-icon> 
                mdi-plus
            </v-icon>
        </v-btn>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import ValorantAgentChooser from '@client/vue/utility/valorant/ValorantAgentChooser.vue'

@Component({
    components: {
        ValorantAgentChooser
    }
})
export default class ValorantTeamCompositionChooser extends Vue {
    @Prop({required: true})
    value!: string[][]

    @Prop({default: 'Agents'})
    label!: string

    onChangeAgent(idx: number, agents: string[]) {
        let comp = [...this.value]
        if (idx >= 0 && idx < comp.length) {
            comp[idx] = [...agents]
        }
        this.$emit('input', comp)
    }

    addToComp() {
        let val = [...this.value, []]
        this.$emit('input', val)
    }

    removeFromComp() {
        if (this.value.length === 0) {
            return
        }

        let val = [...this.value]
        val.pop()
        this.$emit('input', val)
    }
}

</script>

<style scoped>

.member:not(:last-child) {
    margin-right: 4px;
}

</style>