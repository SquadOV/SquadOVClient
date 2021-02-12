<template>
    <div class="status-window" v-if="hasStatus">
        <div class="d-flex align-center status-header">
            <span class="mx-2" :style="statusStyle"></span>
            <span class="text-h6">{{ statusText }}</span>
            <v-spacer></v-spacer>
            <v-btn
                class="mx-1"
                x-small
                icon
                @click="goToStatusPage"
            >
                <v-icon>
                    mdi-open-in-new
                </v-icon>
            </v-btn>
        </div>
        <v-divider></v-divider>

        <div
            class="d-flex align-center py-1"
            v-for="incident in incidents"
            :key="incident.id"
        >
            <span class="mx-2" :style="incidentStyle(incident)"></span>
            <span class="text-caption">{{ incident.name }} ({{ incidentStatus(incident) }}) </span>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import * as color from '@client/js/color'
import { openUrlInBrowser } from '@client/js/external'

//@ts-ignore
const StatusPage = window.StatusPage
const StatusPageId = 'mnzh6hjndkn0'

interface Incident {
    id: string
    impact: string
    name: string
    status: string
    started_at: Date
}

@Component
export default class StatusDisplay extends Vue {
    indicator: string = 'none'
    incidents: Incident[] = []
    itvl: number = 0

    goToStatusPage() {
        openUrlInBrowser('https://squadov.statuspage.io')
    }

    get hasStatus(): boolean {
        return this.indicator != 'none'
    }

    get statusStyle(): any {
        let statusColor: color.Color
        if (this.indicator == 'none') {
            statusColor = color.getStatusOkColor()
        } else if (this.indicator == 'minor') {
            statusColor = color.getStatusMinorColor()
        } else if (this.indicator == 'major') {
            statusColor = color.getStatusMajorColor()
        } else {
            statusColor = color.getStatusCriticalColor()
        }
        
        return {
            height: '24px',
            width: '24px',
            border: '2px solid black',
            'border-radius': '50%',
            display: 'inline-block',
            'background-color': color.colorToCssString(statusColor),
        }
    }

    get statusText(): string {
        if (this.indicator == 'none') {
            return 'No Problems'
        } else if (this.indicator == 'minor') {
            return 'Minor Service Issue'
        } else if (this.indicator == 'major') {
            return 'Major Service Issue'
        } else if (this.indicator == 'critical') {
            return 'Critical Service Issue'
        }
        return 'Unknown Status'
    }

    incidentStyle(i: Incident): any {
        let statusColor: color.Color
        if (i.status == 'resolved' || i.status == 'postmortem') {
            statusColor = color.getStatusOkColor()
        } else if (i.status == 'monitoring') {
            statusColor = color.getStatusMinorColor()
        } else if (i.status == 'identified') {
            statusColor = color.getStatusMajorColor()
        } else {
            statusColor = color.getStatusCriticalColor()
        }

        return {
            height: '16px',
            width: '16px',
            border: '2px solid black',
            'border-radius': '50%',
            display: 'inline-block',
            'background-color': color.colorToCssString(statusColor),
        }
    }

    incidentStatus(i: Incident): string {
        if (i.status == 'resolved' || i.status == 'postmortem') {
            return 'Resolved'
        } else if (i.status == 'monitoring') {
            return 'Monitoring'
        } else if (i.status == 'identified') {
            return 'Identified'
        } else if (i.status == 'investigating') {
            return 'Investigating'
        }
        return 'Unknown Status'
    }

    get sp() {
        //@ts-ignore
        return new StatusPage.page({ page: StatusPageId })
    }

    onIndicatorChange() {
        if (this.hasStatus) {
            //@ts-ignore
            this.sp.incidents({
                filter: 'unresolved',
                success: (data: any) => {
                    this.incidents = data.incidents
                    this.incidents.forEach((ele: Incident) => {
                        ele.started_at = new Date(ele.started_at)
                    })

                    this.incidents.sort((a: Incident, b: Incident) => {
                        return b.started_at.getTime() - a.started_at.getTime()
                    })
                }
            })
        } else {
            this.incidents = []
        }
    }

    refreshStatus() {
        //@ts-ignore
        this.sp.status({
            success: (data: any) => {
                this.indicator = data.status.indicator
                // Call this manually since we want to refresh the incidents
                // every time we tick and not only when the status changes.
                this.onIndicatorChange()
            }
        })
    }

    mounted() {
        this.refreshStatus()
        this.itvl = window.setInterval(() => {
            this.refreshStatus()
        }, 30000)
    }

    beforeDestroy() {
        window.clearInterval(this.itvl)
    }
}

</script>

<style scoped>

.status-window {
    border: 1px solid red;
    border-radius: 5px;
}

.status-header {
    background-color: rgba(255, 255, 255, 0.1);
}

</style>