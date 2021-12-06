<template>
    <div class="d-flex align-center flex-wrap">
        <tag-display
            v-for="(tag, index) in displayTags"
            :tag="tag"
            @update:tag="updateTag(index, arguments[0])"
            class="mr-2"
            :key="`tag-${index}`"
        >
        </tag-display>

        <v-menu
            v-model="menuShow"
            max-width="400px"
            offset-y
            :close-on-content-click="false"
        >
            <template v-slot:activator="{on, attrs}">
                <v-btn
                    color="primary"
                    text
                    small
                    :loading="tagOpInProgress"
                    v-bind="attrs"
                    v-on="on"
                >
                    <v-icon>
                        mdi-plus
                    </v-icon>

                    Tag
                </v-btn>
            </template>

            <v-card>
                <tag-creator v-model="tagsToAdd"></tag-creator>
                <v-card-actions>
                    <v-spacer></v-spacer>
                    <v-btn
                        color="success"
                        @click="saveTags"
                    >
                        Save
                    </v-btn>
                </v-card-actions>
            </v-card>
        </v-menu>

        <v-snackbar
            v-model="addError"
            :timeout="5000"
            color="error"
        >
            Failed to add tags to VOD.
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { VodTag } from '@client/js/squadov/vod'
import TagDisplay from '@client/vue/utility/vods/TagDisplay.vue'
import TagCreator from '@client/vue/utility/vods/TagCreator.vue'
import { apiClient, ApiData } from '@client/js/api'
import { compareString } from '@client/js/cmp'

@Component({
    components: {
        TagDisplay,
        TagCreator,
    }
})
export default class BulkTagDisplay extends Vue {
    @Prop({required: true})
    videoUuid!: string

    @Prop({required: true})
    tags!: VodTag[]
    internalTags: VodTag[] = []

    @Prop({default: -1})
    maxTags!: number

    tagOpInProgress: boolean = false
    tagsToAdd: string[] = []
    menuShow: boolean = false
    addError: boolean = false

    get sortedTags(): VodTag[] {
        return this.internalTags.sort((a: VodTag, b: VodTag) => {
            return ((b.isSelf ? 1 : 0) - (a.isSelf ? 1 : 0)) || (b.count - a.count) || compareString(a.tag, b.tag)
        })
    }

    get displayTags(): VodTag[] {
        return this.sortedTags.slice(0, (this.maxTags >= 0) ? this.maxTags : undefined)
    }

    updateTag(idx: number, tag: VodTag) {
        this.internalTags.splice(idx, 1)
        this.internalTags.push(tag)
        this.internalTags = this.internalTags.filter((ele: VodTag) => ele.count > 0)
    }

    saveTags() {
        this.tagOpInProgress = true
        apiClient.addTagsToVod(this.videoUuid, this.tagsToAdd).then((resp: ApiData<VodTag[]>) => {
            let tagMap: Map<string, VodTag> = new Map()
            for (let t of this.internalTags) {
                tagMap.set(t.tag, JSON.parse(JSON.stringify(t)))
            }

            for (let t of resp.data) {
                tagMap.set(t.tag, t)
            }

            this.internalTags = Array.from(tagMap.values()).filter((ele: VodTag) => ele.count > 0)
            this.tagsToAdd = []
            this.menuShow = false
        }).catch((err: any) => {
            console.error('Failed to add tags: ', err)
        }).finally(() => {
            this.tagOpInProgress = false
        })
    }

    mounted() {
        this.internalTags = JSON.parse(JSON.stringify(this.tags))
    }
}

</script>