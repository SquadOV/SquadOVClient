<template>
    <div>
        <a :href="url" target="_blank">
            <div class="d-flex align-center">
                <v-img
                    :src="icon"
                    width="16px"
                    height="16px"
                    contain
                >
                </v-img>

                <span class="ml-1 text-caption text-uppercase white--text">
                    {{ label }}
                </span>
            </div>
        </a>
    </div>
</template>

<script lang="ts">

import CommonComponent from '@client/vue/CommonComponent'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'

@Component
export default class GenericSocialLink extends CommonComponent {
    @Prop({required: true})
    url!: string

    @Prop({required: true})
    label!: string

    @Prop({required: true})
    icon!: string

    mounted() {
        this.forceLinksToOpenInBrowser()
    }
}

</script>

<style scoped>

</style>