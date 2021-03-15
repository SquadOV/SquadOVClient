<template>
    <div class="d-flex align-center">
        <time-select-filter-ui
            :value="start"
            @input="onChangeStart"
        >
        </time-select-filter-ui>

        <div class="mx-1">to</div>

        <time-select-filter-ui
            :value="end"
            @input="onChangeEnd"
        >
        </time-select-filter-ui>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import TimeSelectFilterUi from '@client/vue/utility/squadov/filters/TimeSelectFilterUi.vue'

@Component({
    components: {
        TimeSelectFilterUi
    }
})
export default class TimeRangeFilterUi extends Vue {
    @Prop({type: Number})
    start!: number | undefined

    @Prop({type: Number})
    end!: number | undefined

    onChange() {
        this.$emit('on-change')
    }

    onChangeStart(v: number | undefined) {
        this.$emit('update:start', v)
        this.onChange()
    }

    onChangeEnd(v: number | undefined) {
        this.$emit('update:end', v)
        this.onChange()
    }
}

</script>