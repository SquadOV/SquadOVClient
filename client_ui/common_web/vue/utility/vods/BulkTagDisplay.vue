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

    @Prop({default: -1})
    maxTags!: number

    tagOpInProgress: boolean = false
    tagsToAdd: string[] = []
    menuShow: boolean = false
    addError: boolean = false

    get sortedTags(): VodTag[] {
        return this.tags.sort((a: VodTag, b: VodTag) => {
            return ((b.isSelf ? 1 : 0) - (a.isSelf ? 1 : 0)) || (b.count - a.count)
        })
    }

    get displayTags(): VodTag[] {
        return this.sortedTags.slice(0, (this.maxTags >= 0) ? this.maxTags : undefined)
    }

    updateTag(idx: number, tag: VodTag) {
        let newTags = [...this.displayTags]
        newTags.splice(idx, 1)
        newTags.push(tag)
        this.$emit('update:tags', newTags)
    }

    saveTags() {
        this.tagOpInProgress = true
        apiClient.addTagsToVod(this.videoUuid, this.tagsToAdd).then((resp: ApiData<VodTag[]>) => {
            let tagMap: Map<string, VodTag> = new Map()
            for (let t of this.tags) {
                tagMap.set(t.tag, JSON.parse(JSON.stringify(t)))
            }

            for (let t of resp.data) {
                tagMap.set(t.tag, t)
            }

            this.$emit('update:tags', Array.from(tagMap.values()))
            this.tagsToAdd = []
            this.menuShow = false
        }).catch((err: any) => {
            console.error('Failed to add tags: ', err)
        }).finally(() => {
            this.tagOpInProgress = false
        })
    }
}

</script>