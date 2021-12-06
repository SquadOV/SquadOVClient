<template>
    <div class="d-flex align-center">
        <v-img
            class="mr-4"
            :src="imageSrc"
            max-width="24"
            width="24"
            max-height="24"
            height="24"
            contain
        >
        </v-img>

        {{ record.name }}
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { ProcessRecord } from '@client/js/system/settings'

@Component
export default class ProcessRecordDisplay extends Vue {
    @Prop({required: true})
    record!: ProcessRecord

    get imageSrc(): string {
        return `data:image/x-icon;base64,${this.record.ico}`
    }
}

</script>