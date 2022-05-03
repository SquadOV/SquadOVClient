<template>
    <div class="d-flex align-center">
        <div class="mr-8" v-if="!!label">
            {{ label }}:
        </div>

        <v-text-field
            :value="keybindStr"
            class="flex-grow-0 mr-2"
            solo
            single-line
            hide-details
            readonly
            style="width: 500px;"
        >
            <template v-slot:append>
                <v-btn class="primary" v-if="!record" @click="startRecord">
                    Edit Keybind
                </v-btn>

                <v-btn class="error" @click="stopRecord" v-else>
                    Stop Recording
                </v-btn>
            </template>
        </v-text-field>

        <template v-if="allowHold">
            <v-checkbox
                class="ma-0"
                :input-value="value.mustHold"
                @change="$emit('input', { keys: value.keys, mustHold: arguments[0], holdSeconds: value.holdSeconds })"
                hide-details
                label="Hold"
            >
                <template v-slot:append>
                    <v-tooltip bottom max-width="450px">
                        <template v-slot:activator="{on, attrs}">
                            <v-icon v-on="on" v-bind="attrs">
                                mdi-help-circle
                            </v-icon>
                        </template>

                        Whether to activate this keybind only after it has been held for a certain amount of time.
                    </v-tooltip>
                </template>
            </v-checkbox>

            <v-slider
                v-if="value.mustHold"
                :value="value.holdSeconds"
                @input="$emit('input', { keys: value.keys, mustHold: value.mustHold, holdSeconds: arguments[0] })"
                inverse-label
                :label="`${value.holdSeconds} seconds`"
                :min="1"
                :max="5"
                :step="1"
                hide-details
            >
                <template v-slot:prepend>
                    Duration (seconds): 
                </template>
            </v-slider>
        </template>
    </div>    
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { Keybind } from '@client/js/system/keybinds'
import KbManager, { KeybindRecordingSession } from '@client/js/system/keybinds'

@Component
export default class KeybindSettingsItem extends Vue {
    @Prop({required: true})
    value!: Keybind

    @Prop()
    label!: string | undefined

    @Prop({type: Boolean, default: false})
    allowHold!: boolean

    keybindStr: string = ''
    record: KeybindRecordingSession | null = null

    @Watch('record', {deep: true})
    @Watch('value')
    refreshString() {
        KbManager.keybindToString(!!this.record ? this.record.keybind : this.value.keys).then((resp: string) => {
            this.keybindStr = resp
        })
    }

    startRecord() {
        if (!!this.record) {
            return
        }

        this.$emit('input', {
            keys: [],
            mustHold: this.value.mustHold,
            holdSeconds: this.value.holdSeconds,
        })
        this.record = Vue.observable(new KeybindRecordingSession())
    }

    stopRecord() {
        if (!this.record) {
            return
        }

        this.$emit('input', {
            keys: this.record.keybind,
            mustHold: this.value.mustHold,
            holdSeconds: this.value.holdSeconds,
        })
        this.record.close()
        this.record = null
    }

    mounted() {
        this.refreshString()
    }
}

</script>