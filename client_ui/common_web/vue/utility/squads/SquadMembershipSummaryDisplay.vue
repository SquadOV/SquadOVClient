<template>
    <v-card class="full-width" :elevation="2" :to="to">
        <div class="d-flex">
            <div>
                <v-card-title>
                    {{ membership.squad.squadName }}
                </v-card-title>

                <v-card-subtitle>
                    {{ membership.squad.squadGroup }} ({{ roleName }})
                </v-card-subtitle>
            </div>

            <v-spacer></v-spacer>

            <div class="d-flex align-center mx-4">
                <div>
                    <v-icon>
                        mdi-account-group
                    </v-icon>
                </div>

                <div class="ml-2 member-count-div">
                    {{ membership.squad.memberCount }}
                </div>
            </div>
        </div>
    </v-card>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { SquadMembership, SquadRole } from '@client/js/squadov/squad'
import * as pi from '@client/js/pages'

@Component
export default class SquadMembershipSummaryDisplay extends Vue {
    @Prop({required: true})
    membership!: SquadMembership

    get roleName() : string {
        return SquadRole[this.membership.role]
    }

    get to(): any {
        return {
            name: pi.SingleSquadPageId,
            params: {
                squadId: this.membership.squad.id
            }
        }
    }
}

</script>

<style scoped>

.member-count-div {
    transform: translateY(2px);
}

</style>