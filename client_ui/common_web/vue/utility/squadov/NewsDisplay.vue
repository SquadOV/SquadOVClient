<template>
    <div :style="newsWindowStyle">
        <v-dialog
            v-model="showNews"
            max-width="60%"
        >
            <single-news-display
                :news="selected"
                @on-close="showNews = false"
            >
            </single-news-display>
        </v-dialog>

        <div class="d-flex align-center news-header pa-1">
            <v-icon class="mr-1">
                mdi-newspaper
            </v-icon>
            <span class="text-h6">Recent Updates</span>
        </div>
        <v-divider></v-divider>
        <loading-container :is-loading="!newsIndex">
            <template v-slot:default="{ loading }">
                <v-list dense three-line v-if="!loading">
                    <v-list-item
                        v-for="(news, idx) in newsIndex"
                        :key="idx"
                        @click="selectNews(news)"
                    >
                        <v-list-item-avatar>
                            <v-icon>
                                {{ typeToIcon(news.type) }}
                            </v-icon>
                        </v-list-item-avatar>

                        <v-list-item-content>
                            <v-list-item-title>{{ news.title }} ({{ newsTime(news) }}) </v-list-item-title>
                            <v-list-item-subtitle v-text="news.description"></v-list-item-subtitle>
                        </v-list-item-content>
                    </v-list-item>
                </v-list>
            </template>
        </loading-container>
    </div>
</template>

<script lang="ts">

import Component, {mixins} from 'vue-class-component'
import CommonComponent from '@client/vue/CommonComponent'
import axios from 'axios'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import SingleNewsDisplay from '@client/vue/utility/squadov/SingleNewsDisplay.vue'
import { ApiData } from '@client/js/api'
import { NewsIndex } from '@client/js/squadov/news'
import { daysAgo } from '@client/js/time'

@Component({
    components: {
        LoadingContainer,
        SingleNewsDisplay
    }
})
export default class NewsDisplay extends mixins(CommonComponent) {
    newsIndex: NewsIndex[] | null = null
    selected: NewsIndex | null = null
    showNews: boolean = false
    itvl: number = 0

    selectNews(n: NewsIndex) {
        this.sendAnalyticsEvent(this.AnalyticsCategory.Button, this.AnalyticsAction.ReadNewsUpdate, n.title, 0)
        
        this.showNews = true
        this.selected = n
    }

    newsTime(n: NewsIndex): string {
        return daysAgo(new Date(n.timestamp))
    }

    typeToIcon(t: string): string {
        if (t == 'update') {
            return 'mdi-update'
        } else if (t == 'hotfix') {
            return 'mdi-fire'
        } else {
            return 'mdi-help-circle'
        }
    }

    refreshNews() {
        axios.get(`https://us-central1.content.squadov.gg/news/index.json?t=${new Date().getTime()}`).then((resp: ApiData<NewsIndex[]>) => {
            this.newsIndex = resp.data
        }).catch((err: any) => {
            console.log('Failed to grab news: ', err)
        })
    }

    mounted() {
        this.refreshNews()
        this.itvl = window.setInterval(() => {
            this.refreshNews()
        }, 600000)
    }

    beforeDestroy() {
        if (!!this.itvl) {
            window.clearInterval(this.itvl)
        }
    }

    get newsWindowStyle(): any {
        return {
        }
    }
}

</script>

<style scoped>

.news-header {
    background-color: rgba(255, 255, 255, 0.1);
}

</style>