<template>
    <div class="py-4 px-16">
        <NuxtChild></NuxtChild>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'

@Component
export default class UserManual extends Vue {
    get layout(): string {
        return 'user-manual'
    }

    head() : any {
        return {
            title: 'SquadOV - User Manual',
            htmlAttrs: {
                lang: 'en'
            },
            meta: [
                {
                    charset: 'utf-8'
                },
                {
                    name: 'viewport',
                    content: 'width=device-width, initial-scale=1'
                },
                {
                    hid: 'description',
                    name: 'description',
                    content: "All the information you'll ever need on how to use SquadOV."
                }
            ]
        }
    }
}

</script>