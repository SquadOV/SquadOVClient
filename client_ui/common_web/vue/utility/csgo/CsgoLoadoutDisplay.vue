<template>
    <div>
        <!-- First row: Primary weapon, sidearm, armor, defuser -->
        <div class="d-flex align-end">
            <csgo-weapon-icon
                :height="36"
                :weapon="primaryWeapon"
            >
            </csgo-weapon-icon>

            <template v-if="armor > 0">
                <v-img
                    class="ml-1"
                    v-if="hasHelmet"
                    :width="16"
                    :max-width="16"
                    :height="18"
                    :max-height="18"
                    :src="$root.generateAssetUri('assets/csgo/misc/icon-armor-helmet.png')"
                    contain
                >
                </v-img>

                <v-img
                    class="ml-1"
                    v-else
                    :width="16"
                    :max-width="16"
                    :height="18"
                    :max-height="18"
                    :src="$root.generateAssetUri('assets/csgo/misc/icon-shield.png')"
                    contain
                >
                </v-img>
            </template>
            
            <v-img
                class="ml-1"
                v-if="hasDefuse"
                :width="20"
                :max-width="20"
                :height="18"
                :max-height="18"
                :src="$root.generateAssetUri('assets/csgo/misc/icon-defuser.png')"
                contain
            >
            </v-img>
        </div>

        <!-- Second row: Side arm Grenades -->
        <div class="d-flex"> 
            <csgo-weapon-icon
                :height="20"
                :weapon="sidearmWeapon"
                v-if="primaryWeapon !== sidearmWeapon"
            >
            </csgo-weapon-icon>

            <csgo-weapon-icon
                v-for="(weapon, idx) in secondaryWeapons"
                :key="idx"
                :weapon="weapon"
                :height="18"
            >
            </csgo-weapon-icon>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { CsgoWeapon, getCsgoPrimaryWeapon, getCsgoSidearmWeapon, getCsgoSecondaryWeapons } from '@client/js/csgo/events'
import CsgoWeaponIcon from '@client/vue/utility/csgo/CsgoWeaponIcon.vue'

@Component({
    components: {
        CsgoWeaponIcon
    }
})
export default class CsgoLoadoutDisplay extends Vue {
    CsgoWeapon = CsgoWeapon
    @Prop({required: true})
    weapons!: CsgoWeapon[] 

    @Prop({required: true})
    hasDefuse!: boolean

    @Prop({required: true})
    hasHelmet!: boolean

    @Prop({required: true})
    armor!: number

    get primaryWeapon(): CsgoWeapon {
        return getCsgoPrimaryWeapon(this.weapons)
    }

    get sidearmWeapon(): CsgoWeapon {
        return getCsgoSidearmWeapon(this.weapons)
    }

    get secondaryWeapons(): CsgoWeapon[] {
        return getCsgoSecondaryWeapons(this.weapons)
    }
}

</script>