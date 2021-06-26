import Vue from 'vue'
import Component from 'vue-class-component'
import { AnalyticsContainer, getAnalyticsContainer } from '@client/js/analytics/container'
import { AnalyticsCategory, AnalyticsAction } from '@client/js/analytics/events'

@Component
export default class CommonComponent extends Vue {
    isActive: boolean = true

    AnalyticsCategory = AnalyticsCategory
    AnalyticsAction = AnalyticsAction

    get analytics(): AnalyticsContainer | null {
        return getAnalyticsContainer()
    }

    sendAnalyticsEvent(category: AnalyticsCategory, action: AnalyticsAction, label: string, value: number) {
        if (!!this.analytics) {
            this.analytics.event(this.$route, category, action, label, value)
        }
    }

    activated() {
        this.isActive = true
    }

    deactivated() {
        this.isActive = false
    }
}