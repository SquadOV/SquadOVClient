<template>
    <div>
        <v-menu
            v-model="showSelector"
            offset-y
            :close-on-content-click="false"
        >
            <template v-slot:activator="{on}">
                <div
                    class="activator"
                    @click="showSelector = true"
                >
                    <valorant-agent-icon
                        :agent="agentToDisplay"
                        v-if="value.length <= 1"
                    >
                    </valorant-agent-icon>

                    <valorant-agent-role-icon
                        :role="roleToDisplay"
                        v-else
                    >
                    </valorant-agent-role-icon>
                </div>
            </template>

            <div class="pa-4 selector">
                <!-- First row to make it easy to select all agents in a single role -->
                <div class="d-flex align-center">
                    <div
                        class="choice"
                        v-for="(role, idx) in ALL_VALORANT_AGENT_ROLES"
                        :key="`role-${idx}`"
                        v-ripple
                        @click="selectRole(role)"
                    >
                        <valorant-agent-role-icon
                            :role="role"
                        >
                        </valorant-agent-role-icon>
                    </div>
                </div>

                <!-- Next row to select agents individually -->
                <div class="d-flex align-center flex-wrap mt-2">
                    <div
                        :class="`choice ${value.includes(agent) ? 'selected' : ''}`"
                        v-for="(agent, idx) in ALL_VALORANT_AGENTS"
                        :key="`agent-${idx}`"
                        v-ripple
                        @click="toggleAgent(agent)"
                    >
                        <valorant-agent-icon
                            :agent="agent"
                        >
                        </valorant-agent-icon>
                    </div>
                </div>
            </div>
        </v-menu>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import ValorantAgentIcon from '@client/vue/utility/valorant/ValorantAgentIcon.vue'
import ValorantAgentRoleIcon from '@client/vue/utility/valorant/ValorantAgentRoleIcon.vue'
import { getProbableRoleFromMultipleAgents, ValorantAgentRole, ALL_VALORANT_AGENT_ROLES, getAgentsFromRole, getAvailableAgents } from '@client/js/valorant/valorant_utility'
import { compareString } from '@client/js/cmp'
import { getValorantContent } from '@client/js/valorant/valorant_content'

@Component({
    components: {
        ValorantAgentIcon,
        ValorantAgentRoleIcon,
    }
})
export default class ValorantAgentChooser extends Vue {
    ALL_VALORANT_AGENTS = getAvailableAgents().sort((a: string, b: string) => {
        return compareString(getValorantContent(null).agentIdToName(a), getValorantContent(null).agentIdToName(b))
    })
    ALL_VALORANT_AGENT_ROLES = ALL_VALORANT_AGENT_ROLES.filter((ele) => ele !== ValorantAgentRole.Unknown)

    @Prop({required: true})
    value!: string[]

    showSelector: boolean = false

    get agentToDisplay(): string {
        if (this.value.length === 0) {
            return ''
        } else {
            return this.value[0]
        }
    }

    get roleToDisplay(): ValorantAgentRole {
        if (this.value.length === 0) {
            return ValorantAgentRole.Unknown
        } else {
            return getProbableRoleFromMultipleAgents(this.value)
        }
    }

    selectRole(role: ValorantAgentRole) {
        this.$emit('input', getAgentsFromRole(role))
    }

    toggleAgent(agent: string) {
        let existing = new Set([...this.value])
        if (existing.has(agent)) {
            existing.delete(agent)
        } else {
            existing.add(agent)
        }
        this.$emit('input', Array.from(existing.values()))
    }
}

</script>

<style scoped>

.activator {
    cursor: pointer;
}

.selector {
    background-color: #1e1e1e;
}

.selected {
    border: 1px solid yellow;
}

.choice {
    cursor: pointer;
}

.choice:not(:last-child) {
    margin-right: 4px;
}

</style>