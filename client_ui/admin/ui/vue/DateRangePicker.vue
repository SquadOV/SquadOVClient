<template>
    <div class="d-flex align-center">
        <v-menu
            v-model="showStartSelector"
            offset-y
            :close-on-content-click="false"
        >
            <template v-slot:activator="{on}">
                <v-text-field
                    :value="startDateStr"
                    hide-details
                    readonly
                    prepend-icon="mdi-calendar"
                    outlined
                    single-line
                    @click="showStartSelector = true"
                >
                </v-text-field>
            </template>
            
            <v-date-picker
                :value="startDateIso"
                @input="updateDate(arguments[0], true)"
            >
            </v-date-picker>
        </v-menu>

        <div class="text-overline mx-2">
            to
        </div>

        <v-menu
            v-model="showEndSelector"
            offset-y
            :close-on-content-click="false"
        >
            <template v-slot:activator="{on}">
                <v-text-field
                    :value="endDateStr"
                    hide-details
                    readonly
                    prepend-icon="mdi-calendar"
                    outlined
                    single-line
                    @click="showEndSelector = true"
                >
                </v-text-field>
            </template>
            
            <v-date-picker
                :value="endDateIso"
                @input="updateDate(arguments[0], false)"
            >
            </v-date-picker>
        </v-menu>

        
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import format from 'date-fns/format'

@Component
export default class DateRangePicker extends Vue {
    @Prop({required: true})
    startDate!: Date
    showStartSelector: boolean = false

    @Prop({required: true})
    endDate!: Date
    showEndSelector: boolean = false

    updateDate(dd: string, isStart: boolean) {
        let tokens = dd.split('-')
        let newDate = new Date()
        newDate.setUTCFullYear(parseInt(tokens[0]), parseInt(tokens[1])-1, parseInt(tokens[2]))
        newDate.setUTCHours(0, 0, 0, 0)
        if (isStart) {
            this.$emit('update:startDate', newDate)
        } else {
            this.$emit('update:endDate', newDate)
        }
    }

    get startDateStr(): string {
        return `${this.startDate.getUTCMonth()+1}/${this.startDate.getUTCDate()}/${this.startDate.getUTCFullYear()}`
    }

    get startDateIso(): string {
        return this.startDate.toISOString().substr(0, 10)
    }

    get endDateStr(): string {
        return `${this.endDate.getUTCMonth()+1}/${this.endDate.getUTCDate()}/${this.endDate.getUTCFullYear()}`
    }

    get endDateIso(): string {
        return this.endDate.toISOString().substr(0, 10)
    }
}

</script>