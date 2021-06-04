<template>
    <div class="status-window" v-if="hasDisplayStatus">
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
            v-for="mnt in allMaintenance"
            :key="mnt.id"
        >
            <span class="mx-2" :style="maintenanceStyle(mnt)"></span>
            <span class="text-caption">MAINTENANCE: {{ mnt.name }} ({{ maintenanceStatus(mnt) }}) </span>
        </div>

        <div
            class="d-flex align-center py-1"
            v-for="incident in incidents"
            :key="incident.id"
        >
            <span class="mx-2" :style="incidentStyle(incident)"></span>
            <span class="text-caption">INCIDENT: {{ incident.name }} ({{ incidentStatus(incident) }}) </span>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
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

interface Maintenance {
    id: string
    name: string
    scheduled_for: Date
    scheduled_until: Date
    status: string
}

@Component
export default class StatusDisplay extends Vue {
    indicator: string = 'none'
    incidents: Incident[] = []
    upcomingMaintenance: Maintenance[] = []
    activeMaintenance: Maintenance[] = []
    itvl: number = 0

    get allMaintenance(): Maintenance[] {
        return [...this.activeMaintenance, ...this.upcomingMaintenance]
    }

    goToStatusPage() {
        openUrlInBrowser('https://squadov.statuspage.io')
    }

    get hasStatus(): boolean {
        return (this.indicator != 'none')
    }

    get hasDisplayStatus(): boolean {
        return this.incidents.length > 0 || this.upcomingMaintenance.length > 0 || this.activeMaintenance.length > 0
    }

    get statusStyle(): any {
        let statusColor: string
        if (this.activeMaintenance.length > 0) {
            statusColor = 'color-status-maintenance'
        } else if (this.indicator == 'minor') {
            statusColor = 'color-status-minor'
        } else if (this.indicator == 'major') {
            statusColor = 'color-status-major'
        } else if (this.upcomingMaintenance.length > 0) {
            statusColor ='color-status-maintenance'
        } else if (this.indicator == 'none') {
            statusColor = 'color-status-ok'
        } else {
            statusColor = 'color-status-critical'
        }
        
        return {
            height: '24px',
            width: '24px',
            border: '2px solid black',
            'border-radius': '50%',
            display: 'inline-block',
            'background-color':  `var(--${statusColor})`,
        }
    }

    get statusText(): string {
        if (this.activeMaintenance.length > 0) {
            return 'Active Maintenance'
        } else if (this.indicator == 'minor') {
            return 'Minor Service Issue'
        } else if (this.indicator == 'major') {
            return 'Major Service Issue'
        } else if (this.indicator == 'critical') {
            return 'Critical Service Issue'
        } else if (this.upcomingMaintenance.length > 0) {
            return 'Upcoming Maintenance'
        } else if (this.indicator == 'none') {
            return 'No Problems'
        } 
        return 'Unknown Status'
    }

    maintenanceStatus(i: Maintenance): string {
        if (i.status == 'in_progress' || i.status == 'verifying') {
            return 'In Progress'
        } else if (i.status == 'scheduled') {
            return 'Scheduled'
        }
        return 'Unknown Status'
    }

    maintenanceStyle(i: Incident): any {
        let statusColor: string
        if (i.status == 'in_progress' || i.status == 'verifying') {
            statusColor = 'color-status-critical'
        } else if (i.status == 'scheduled') {
            statusColor = 'color-status-ok'
        } else {
            statusColor = 'color-status-maintenance'
        }

        return {
            height: '16px',
            width: '16px',
            border: '2px solid black',
            'border-radius': '50%',
            display: 'inline-block',
            'background-color':  `var(--${statusColor})`,
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

    incidentStyle(i: Maintenance): any {
        let statusColor: string
        if (i.status == 'resolved' || i.status == 'postmortem') {
            statusColor = 'color-status-ok'
        } else if (i.status == 'monitoring') {
            statusColor = 'color-status-minor'
        } else if (i.status == 'identified') {
            statusColor = 'color-status-major'
        } else {
            statusColor = 'color-status-critical'
        }

        return {
            height: '16px',
            width: '16px',
            border: '2px solid black',
            'border-radius': '50%',
            display: 'inline-block',
            'background-color':  `var(--${statusColor})`,
        }
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

        //@ts-ignore
        this.sp.scheduled_maintenances({
            filter: 'upcoming',
            success: (data: any) => {
                this.upcomingMaintenance = data.scheduled_maintenances
            } 
        })

        //@ts-ignore
        this.sp.scheduled_maintenances({
            filter: 'active',
            success: (data: any) => {
                this.activeMaintenance = data.scheduled_maintenances
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
    border: 1px solid var(--color-bottom-place);
    border-radius: 5px;
}

.status-header {
    background-color: rgba(255, 255, 255, 0.1);
}

</style>