<template>
    <v-card v-if="!!news">
        <v-card-title>
            <div class="d-flex full-width align-end">
                <span class="mr-2">{{ news.title }}</span>
                <v-spacer></v-spacer>
                <span class="text-caption">{{ newsTime }}</span>
            </div>
        </v-card-title>
        <v-divider></v-divider>

        <loading-container :is-loading="!markdownText">
            <template v-slot:default="{ loading }">
                <v-card-text class="update-container" v-if="!loading" v-html="newsHtml">
                </v-card-text>
            </template>
        </loading-container>

        <v-card-actions>
            <v-spacer></v-spacer>
            <v-btn
                @click="$emit('on-close')"
                color="success"
            >
                Close
            </v-btn>
        </v-card-actions>
    </v-card>    
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { NewsIndex } from '@client/js/squadov/news'
import { ApiData } from '@client/js/api'
import { standardFormatTime } from '@client/js/time'
import { openUrlInBrowser } from '@client/js/external'

import axios from 'axios'
import marked from 'marked'
import DOMPurify from 'dompurify'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'

@Component({
    components: {
        LoadingContainer
    }
})
export default class SingleNewsDisplay extends Vue {
    @Prop({required: true})
    news!: NewsIndex | null
    markdownText: string | null = null

    forceLinksToOpenInBrowser() {
/// #if DESKTOP
        this.$el.querySelectorAll('a').forEach((e: HTMLAnchorElement) => {
            if (e.target != '_blank') {
                return
            }
            e.addEventListener('click', (e: MouseEvent) => {
                if (!!e.target) {
                    //@ts-ignore
                    openUrlInBrowser(e.target.href)
                }
                e.preventDefault()
            })
        })
/// #endif
    }

    get newsHtml(): string {
        if (!this.markdownText) {
            return ''
        }

        const renderer = new marked.Renderer()
        const linkRenderer = renderer.link
        renderer.link = (href: string, title: string, text: string) => {
            const html = linkRenderer.call(renderer, href, title, text);
            return html.replace(/<a /, '<a target="_blank" ');
        }

        Vue.nextTick(() => {
            this.forceLinksToOpenInBrowser()
        })
        return DOMPurify.sanitize(marked(this.markdownText, { renderer  }), { ADD_ATTR: ['target'] })
    }

    get newsTime(): string {
        if (!this.news) {
            return ''
        }

        return standardFormatTime(new Date(this.news.timestamp))
    }

    @Watch('news')
    refreshData() {
        this.markdownText = null
        if (!this.news) {
            return
        }

        axios.get(`https://us-central1.content.squadov.gg/news/${this.news.data}`).then((resp: ApiData<string>) => {
            this.markdownText = resp.data
        }).catch((err: any) => {
            console.warn('Failed to grab news item: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>