<template>
    <div>
        <v-menu
            v-model="showSelector"
            offset-y
            :close-on-content-click="false"
        >
            <template v-slot:activator="{on}">
                <div
                    class="activator"
                    @click="showSelector = true"
                >
                    <template v-if="!isRoleMode">
                        <wow-class-spec-icon
                            :spec-id="specToDisplay"
                            :patch="LATEST_RETAIL_PATCH_ID"
                            v-if="value.length <= 1"
                        >
                        </wow-class-spec-icon>

                        <wow-class-icon
                            :class-id="classToDisplay"
                            :patch="LATEST_RETAIL_PATCH_ID"
                            v-else
                        >
                        </wow-class-icon>
                    </template>

                    <wow-role-icon
                        v-else
                        :value="selectedRole"
                    >
                    </wow-role-icon>
                </div>
            </template>

            <div class="pa-4 selector">
                <!-- First row to toggle role mode -->
                <v-checkbox
                    class="mb-2"
                    v-model="isRoleMode"
                    dense
                    hide-details
                    label="Select Roles"
                >
                </v-checkbox>

                <template v-if="!isRoleMode">
                    <!-- Next row to display class options. Only a single class can be selected. -->
                    <v-select
                        class="mt-2"
                        label="Class"
                        :value="classToDisplay"
                        @input="changeClass"
                        :items="classItems"
                        outlined
                        hide-details
                        dense
                    >
                        <template v-slot:item="{ item }">
                            <div class="d-flex full-width align-center">
                                {{ item.text }}

                                <v-spacer></v-spacer>

                                <wow-class-icon
                                    :class-id="item.value"
                                    :patch="LATEST_RETAIL_PATCH_ID"
                                >
                                </wow-class-icon>
                            </div>
                        </template>
                    </v-select>

                    <!-- Last row to display spec options. One or more specs can be selected. -->
                    <v-select
                        class="mt-2"
                        label="Specs"
                        :value="value"
                        @input="$emit('input', arguments[0])"
                        :items="specItems"
                        deletable-chips
                        chips
                        multiple
                        clearable
                        outlined
                        hide-details
                        dense
                    >
                        <template v-slot:item="{ item }">
                            <div class="d-flex full-width align-center">
                                <v-checkbox
                                    class="selection-checkbox"
                                    dense
                                    hide-details
                                    :input-value="value.includes(item.value)"
                                    readonly
                                >
                                </v-checkbox>

                                {{ item.text }}

                                <v-spacer></v-spacer>

                                <wow-class-spec-icon
                                    :spec-id="item.value"
                                    :patch="LATEST_RETAIL_PATCH_ID"
                                >
                                </wow-class-spec-icon>
                            </div>
                        </template>
                    </v-select>
                </template>

                <template v-else>
                    <v-item-group
                        v-model="selectedRole"
                        mandatory
                    >
                        <div class="d-flex align-center">
                            <v-item
                                v-slot="{active, toggle}"
                                v-for="(role, idx) in roleItems"
                                :key="idx"
                            >
                                <div
                                    :class="(active ? 'selected-role' : '') + ' role'"
                                    @click="toggle"
                                >
                                    <wow-role-icon
                                        :value="role"
                                    >
                                    </wow-role-icon>
                                </div>
                            </v-item>
                        </div>
                    </v-item-group>
                </template>
            </div>
        </v-menu>
    </div>    
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { LATEST_RETAIL_PATCH_ID } from '@client/js/wow/staticCache'
import { specIdToClassId, classToSpecIds, ALL_WOW_CLASSES, classToName, specToName, WowRole, roleToSpecIds } from '@client/js/wow/character'
import WowClassSpecIcon from '@client/vue/utility/wow/WowClassSpecIcon.vue'
import WowClassIcon from '@client/vue/utility/wow/WowClassIcon.vue'
import WowRoleIcon from '@client/vue/utility/wow/WowRoleIcon.vue'

@Component({
    components: {
        WowClassSpecIcon,
        WowClassIcon,
        WowRoleIcon,
    }
})
export default class WowSpecChooser extends Vue {
    LATEST_RETAIL_PATCH_ID: string = LATEST_RETAIL_PATCH_ID

    @Prop()
    value!: number[]

    isRoleMode: boolean = false
    selectedRole: WowRole = WowRole.Unknown

    // Need to use the showSelector thing manually because doing v-on="on" on the WowClassSpecIcon
    // component doesn't seem to work in triggering the menu.
    showSelector: boolean = false

    changeClass(classId: number) {
        this.$emit('input', classToSpecIds(classId))
    }

    @Watch('isRoleMode')
    onSwapMode() {
        this.$emit('input', [])
    }

    @Watch('selectedRole')
    onRoleChange(role: WowRole) {
        this.$emit('input', roleToSpecIds(role))
    }

    get roleItems(): any[] {
        return [WowRole.Tank, WowRole.Healer, WowRole.Dps, WowRole.Unknown]
    }

    get classItems(): any[] {
        return ALL_WOW_CLASSES.map((ele: number) => {
            return {
                value: ele,
                text: classToName(ele),
            }
        })
    }

    get specItems(): any[] {
        return classToSpecIds(this.classToDisplay).map((ele: number) => {
            return {
                value: ele,
                text: specToName(ele),
            }
        })
    }

    get specToDisplay(): number {
        if (this.value.length === 0) {
            return -1
        } else {
            return this.value[0]
        }
    }

    get classToDisplay(): number {
        if (this.value.length === 0) {
            return -1
        } else {
            return specIdToClassId(this.value[0])
        }
    }
}

</script>

<style scoped>

.activator {
    cursor: pointer;
}

.selector {
    background-color: #1e1e1e;
}

.selected-role {
    border: 1px solid yellow;
}

.role {
    cursor: pointer;
}

</style>