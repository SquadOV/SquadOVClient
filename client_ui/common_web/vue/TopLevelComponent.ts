import Vue from 'vue'
import Component from 'vue-class-component'
import LRU from 'lru-cache'

interface CachedScroll {
    left: number
    top: number
}

@Component
export default class TopLevelComponent extends Vue {
    cache: any = null

    restoreScroll(key: string) {
        if (!this.cache) {
            return
        }

        let val: CachedScroll | undefined = this.cache.get(key)
        if (!val) {
            return
        }
        this.$el.scrollLeft = val.left
        this.$el.scrollTop = val.top
    }

    saveScroll(key: string) {
        if (!this.cache) {
            return
        }

        let val: CachedScroll = {
            left: this.$el.scrollLeft,
            top: this.$el.scrollTop
        }
        
        this.cache.set(key, val)
    }

    activated() {
        this.restoreScroll(this.$route.fullPath)
    }

    mounted() {
        this.cache = new LRU(32)
        this.$el.addEventListener('scroll', () => {
            this.saveScroll(this.$route.fullPath)
        })
    }
}