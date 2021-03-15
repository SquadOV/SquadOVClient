<template>
    <div>
        <v-menu
            v-model="showSelector"
            offset-y
            :close-on-content-click="false"
        >
            <template v-slot:activator="{on}">
                <v-text-field
                    :value="valueDisplay"
                    dense
                    hide-details
                    readonly
                    outlined
                    single-line
                    @click="showSelector = true"
                >
                    <template v-slot:append>
                        <v-btn class="close-button" icon @click="onChange(null)">
                            <v-icon>
                                mdi-close
                            </v-icon>
                        </v-btn>
                    </template>
                </v-text-field>
            </template>
            
            <div class="d-flex align-center">
                <v-date-picker
                    class="mr-1"
                    :value="dateIso"
                    @input="changeDate"
                >
                </v-date-picker>

                <v-time-picker
                    :value="timeIso"
                    @input="changeTime"
                    format="24hr"
                    scrollable
                >
                </v-time-picker>
            </div>
        </v-menu>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { standardFormatTime } from '@client/js/time'

@Component
export default class TimeSelectFilterUi extends Vue {
    @Prop({type: Number})
    value!: number | undefined

    showSelector: boolean = false

    onChange(d: Date | null) {
        if (!!d) {
            this.$emit('input', d.getTime())
        } else {
            this.$emit('input', undefined)
        }
    }

    changeDate(d: string) {
        let n : Date = new Date(this.valueDate.getTime())

        let data : string[] = d.split('-')
        n.setFullYear(parseInt(data[0]))
        n.setMonth(parseInt(data[1])-1)
        n.setDate(parseInt(data[2]))
        this.onChange(n)
    }

    changeTime(t: string) {
        let n : Date = new Date(this.valueDate.getTime())

        let data : string[] = t.split(':')
        n.setHours(parseInt(data[0]))
        n.setMinutes(parseInt(data[1]))
        this.onChange(n)
    }

    get valueDate(): Date {
        if (this.value === undefined) {
            return new Date()
        }
        return new Date(this.value)
    }

    get valueDisplay(): string {
        if (this.value === undefined) {
            return 'Whenever'
        }
        return standardFormatTime(this.valueDate)
    }

    get dateIso(): string {
        return this.valueDate.toISOString().substr(0, 10)
    }

    get timeIso(): string {
        return `${this.valueDate.getHours().toString().padStart(2, "0")}:${this.valueDate.getMinutes().toString().padStart(2, "0")}`
    }
}

</script>

<style scoped>

.close-button {
    transform: translateY(-4px);
}

</style>