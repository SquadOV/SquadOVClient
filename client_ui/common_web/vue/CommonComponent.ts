import Vue from 'vue'
import Component from 'vue-class-component'
import { AnalyticsContainer, getAnalyticsContainer } from '@client/js/analytics/container'
import { AnalyticsCategory, AnalyticsAction } from '@client/js/analytics/events'
import { openUrlInBrowser} from '@client/js/external'

@Component
export default class CommonComponent extends Vue {
    isActive: boolean = true

    AnalyticsCategory = AnalyticsCategory
    AnalyticsAction = AnalyticsAction

    get analytics(): AnalyticsContainer | null {
        return getAnalyticsContainer()
    }

    get cleanQuery(): any {
        let q = JSON.parse(JSON.stringify(this.$route.query))
        delete q['at']
        return q
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

    forceLinksToOpenInBrowser() {
/// #if DESKTOP
        this.$el.querySelectorAll('a').forEach((he: HTMLAnchorElement) => {
            he.addEventListener('click', (me: MouseEvent) => {
                openUrlInBrowser(he.href)
                me.preventDefault()
            })
        })
/// #endif
    }
}