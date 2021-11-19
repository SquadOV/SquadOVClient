import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { DataStorageLocation, getDataStorageSingleton } from '@client/js/system/data_storage'

@Component
export default class CommonFilters extends Vue {
    @Prop()
    savedFilterLoc!: DataStorageLocation | null
    savedFilter: any = null

    loadFromLocal() {
        if (this.savedFilterLoc !== null) {
            let storage = getDataStorageSingleton()
            this.savedFilter = storage?.getData(this.savedFilterLoc)
        }
    }

    saveToLocal(data: any) {
        if (this.savedFilterLoc !== null) {
            let storage = getDataStorageSingleton()
            storage?.setData(this.savedFilterLoc, data)
        }
    }

    mounted() {
        this.loadFromLocal()
    }
}