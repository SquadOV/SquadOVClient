<template>
    <v-card>
        <v-card-title>
            Video
        </v-card-title>

        <v-divider></v-divider>

        <v-container fluid>
            <v-row>
                <v-col cols="2">
                    <b>ID</b>
                </v-col>

                <v-col cols="10">
                    {{ vod.videoUuid }}
                </v-col>
            </v-row>

            <v-row>
                <v-col cols="2">
                    <b>User</b>
                </v-col>

                <v-col cols="10">
                    {{ vod.userUuid }}
                </v-col>
            </v-row>

            <v-row>
                <v-col cols="2">
                    <b>Match</b>
                </v-col>

                <v-col cols="10">
                    {{ vod.matchUuid }}
                </v-col>
            </v-row>

            <v-row>
                <v-col cols="2">
                    <b>Start</b>
                </v-col>

                <v-col cols="10">
                    {{ vod.startTime }}
                </v-col>
            </v-row>

            <v-row>
                <v-col cols="2">
                    <b>End</b>
                </v-col>

                <v-col cols="10">
                    <span v-if="!!vod.endTime">{{ vod.endTime }}</span>
                    <span v-else>No End Time (VOD was not completed properly)</span>
                </v-col>
            </v-row>

            <v-row>
                <v-col cols="2">
                    <b>Resolution (FPS)</b>
                </v-col>

                <v-col cols="10">
                    {{ vod.resX }}x{{vod.resY}}@{{vod.fps}}fps
                </v-col>
            </v-row>

            <v-row>
                <v-col cols="2">
                    <b>Is Clip</b>
                </v-col>

                <v-col cols="10">
                    <v-icon color="success" v-if="vod.isClip">
                        mdi-check-circle
                    </v-icon>

                    <v-icon color="error" v-else>
                        mdi-close-circle
                    </v-icon>
                </v-col>
            </v-row>

            <v-row>
                <v-col cols="2">
                    <b>Is Local</b>
                </v-col>

                <v-col cols="10">
                    <v-icon color="success" v-if="vod.isLocal">
                        mdi-check-circle
                    </v-icon>

                    <v-icon color="error" v-else>
                        mdi-close-circle
                    </v-icon>
                </v-col>
            </v-row>

            <v-row>
                <v-col cols="2">
                    <b>Is Processed</b>
                </v-col>

                <v-col cols="10">
                    <v-icon color="success" v-if="vod.isProcessed">
                        mdi-check-circle
                    </v-icon>

                    <v-icon color="error" v-else>
                        mdi-close-circle
                    </v-icon>
                </v-col>
            </v-row>

            <v-row>
                <v-col cols="2">
                    <b>Is Public</b>
                </v-col>

                <v-col cols="10">
                    <v-icon color="success" v-if="!!vod.shareToken">
                        mdi-check-circle
                    </v-icon>

                    <v-icon color="error" v-else>
                        mdi-close-circle
                    </v-icon>
                </v-col>
            </v-row>

            <v-row v-if="!!vod.oembed">
                <v-col cols="2">
                    <b>Public Preview</b>
                </v-col>

                <v-col cols="10">
                    <div v-html="vod.oembed.html"></div>
                </v-col>
            </v-row>
        </v-container>
    </v-card>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { Vod } from '@client/ts/vod'
import { Oembed } from '@client/ts/oembed'

@Component
export default class VodDisplay extends Vue {
    @Prop({required: true})
    vod!: Vod
}

</script>