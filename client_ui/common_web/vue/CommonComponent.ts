import Vue from 'vue'
import Component from 'vue-class-component'
import { AnalyticsContainer, getAnalyticsContainer } from '@client/js/analytics/container'
import { AnalyticsCategory, AnalyticsAction } from '@client/js/analytics/events'

@Component
export default class CommonComponent extends Vue {
    isActive: boolean = true

    AnalyticsCategory = AnalyticsCategory
    AnalyticsAction = AnalyticsAction

    get analytics(): AnalyticsContainer {
        return getAnalyticsContainer()
    }

    sendAnalyticsEvent(category: AnalyticsCategory, action: AnalyticsAction, label: string, value: number) {
        this.analytics.event(this.$route.fullPath, category, action, label, value)
    }

    activated() {
        this.isActive = true
    }

    deactivated() {
        this.isActive = false
    }
}