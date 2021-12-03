<template>
    <div>
        <v-chip @click="toggleTag" ripple small :color="tagColor">
            {{ tag.tag }}
        </v-chip>

        <v-snackbar
            v-model="toggleError"
            :timeout="5000"
            color="error"
        >
            Failed to toggle tag.
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { VodTag } from '@client/js/squadov/vod'
import { apiClient } from '@client/js/api'

@Component
export default class TagDisplay extends Vue {
    @Prop({required: true})
    tag!: VodTag
    toggleError: boolean = false

    get tagColor(): string {
        return this.tag.isSelf ? 'primary' : ''
    }

    toggleTag() {
        let newTag: VodTag = JSON.parse(JSON.stringify(this.tag))

        let promise
        if (newTag.isSelf) {
            promise = apiClient.deleteTagFromVod(this.tag.videoUuid, this.tag.tagId)
        } else {
            promise = apiClient.addTagsToVod(this.tag.videoUuid, [this.tag.tag])
        }

        newTag.isSelf = !newTag.isSelf
        promise.then(() => {
            this.$emit('update:tag', newTag)
        }).catch((err: any) => {
            console.error('Failed to toggle tag: ', err)
            this.toggleError = true
        })
    }
}

</script>