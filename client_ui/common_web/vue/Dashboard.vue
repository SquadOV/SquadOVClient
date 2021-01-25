<template>
    <loading-container :is-loading="!loaded">
        <template v-slot:default="{ loading }">
            <v-container v-if="!loading">
                <v-row>
                    <v-col cols="3">
                        <!-- User and squad -->
                    </v-col>

                    <v-col cols="6">
                        <!-- Recent games -->
                    </v-col>

                    <v-col cols="3">
                        <!-- News and updates -->
                        <status-display></status-display>
                        <news-display></news-display>
                    </v-col>
                </v-row>
            </v-container>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import * as pi from '@client/js/pages'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import StatusDisplay from '@client/vue/utility/squadov/StatusDisplay.vue'
import NewsDisplay from '@client/vue/utility/squadov/NewsDisplay.vue'

@Component({
    components: {
        LoadingContainer,
        StatusDisplay,
        NewsDisplay,
    }
})
export default class Dashboard extends Vue {

    get loaded(): boolean {
        return !!this.$store.state.currentUser
    }

    get logsTo() : any {
        return {
            name: pi.LogPageId,
            params: {
                userId: this.$store.state.currentUser.id
            }
        }
    }

    get performanceTo() : any {
        return {
            name: pi.PerformancePageId,
            params: {
                userId: this.$store.state.currentUser.id
            }
        }
    }
}

</script>

<style scoped>

.choice {
    margin-left: 8px;
    margin-right: 8px;
}

</style>