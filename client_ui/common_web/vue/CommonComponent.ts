import Vue from 'vue'
import Component from 'vue-class-component'

@Component
export default class CommonComponent extends Vue {
    isActive: boolean = true

    activated() {
        this.isActive = true
    }

    deactivated() {
        this.isActive = false
    }
}