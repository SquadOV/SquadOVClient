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
                @input="$emit('update:startDate', new Date(arguments[0]))"
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
                @input="$emit('update:endDate', new Date(arguments[0]))"
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

    get startDateStr(): string {
        return format(this.startDate, 'MM/dd/yyyy')
    }

    get startDateIso(): string {
        return this.startDate.toISOString().substr(0, 10)
    }

    get endDateStr(): string {
        return format(this.endDate, 'MM/dd/yyyy')
    }

    get endDateIso(): string {
        return this.endDate.toISOString().substr(0, 10)
    }
}

</script>