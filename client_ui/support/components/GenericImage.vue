<template>
    <div>
        <a class="text-body-2 image-link" :href="src" target="_blank">
            <div class="d-flex flex-column justify-center align-center">
                <v-img
                    :src="src"
                    :max-width="width"
                    contain
                >
                </v-img>
                <div class="text-body-2">
                    <span class="image-link">{{ caption }}</span>
                </div>
            </div>
        </a>
    </div>
</template>

<script lang="ts">


import Vue from 'vue'
import Component from 'nuxt-class-component'
import { Prop } from 'nuxt-property-decorator'

@Component
export default class GenericImage extends Vue {
    @Prop()
    src!: string

    @Prop()
    caption!: string

    @Prop()
    alt!: string

    @Prop({ default: "40%"})
    width!: string
}

</script>

<style scoped>

.image-link {
    text-decoration: none;
    color: white;
}

</style>