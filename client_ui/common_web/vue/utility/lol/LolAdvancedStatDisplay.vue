<template>
    <div class="d-flex justify-center">
        <v-checkbox
            v-if="typeof(value) === 'boolean'"
            :input-value="value"
            readonly
        >
        </v-checkbox>

        <span v-else>
            {{ value }}
        </span>
    </div>
</template>

<script lang="ts">
import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'

@Component
export default class LolAdvancedStatDisplay extends Vue {
    @Prop({required: true})
    value!: any
}

</script>

<style scoped>

>>>.v-input--selection-controls__input {
    margin-right: 0px !important;
}

</style>