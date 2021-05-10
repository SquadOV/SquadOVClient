<template>
    <v-img
        v-if="isValid"
        :height="height"
        :max-height="height"
        :src="$root.generateAssetUri(src)"
    >
    </v-img>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { CsgoWeapon } from '@client/js/csgo/events'

@Component
export default class CsgoTeamIcon extends Vue {
    @Prop({required: true})
    weapon!: CsgoWeapon

    @Prop({default: 48})
    height!: number

    get weaponIcon(): string {
        switch (this.weapon) {
            case CsgoWeapon.Knife:
                return 'knife'
            case CsgoWeapon.Bomb:
                return 'c4'
            case CsgoWeapon.Taser:
                return 'taser'
            case CsgoWeapon.P2000:
                return 'p2000'
            case CsgoWeapon.Usps:
                return 'usp_silencer'
            case CsgoWeapon.Glock:
                return 'glock'
            case CsgoWeapon.P250:
                return 'p250'
            case CsgoWeapon.FiveSeven:
                return 'fiveseven'
            case CsgoWeapon.Tec9:
                return 'tec9'
            case CsgoWeapon.Cz75:
                return 'cz75a'
            case CsgoWeapon.DuelBerettas:
                return 'elite'
            case CsgoWeapon.Deagle:
                return 'deagle'
            case CsgoWeapon.R8:
                return 'revolver'
            case CsgoWeapon.Mp9:
                return 'mp9'
            case CsgoWeapon.Mac10:
                return 'mac10'
            case CsgoWeapon.PpBizon:
                return 'bizon'
            case CsgoWeapon.Mp7:
                return 'mp7'
            case CsgoWeapon.Ump45:
                return 'ump45'
            case CsgoWeapon.P90:
                return 'p90'
            case CsgoWeapon.Mp5:
                return 'mp5sd'
            case CsgoWeapon.Famas:
                return 'famas'
            case CsgoWeapon.Galil:
                return 'galilar'
            case CsgoWeapon.M4a4:
                return 'm4a1'
            case CsgoWeapon.M4a1s:
                return 'm4a1_silencer'
            case CsgoWeapon.Ak47:
                return 'ak47'
            case CsgoWeapon.Aug:
                return 'aug'
            case CsgoWeapon.Sg553:
                return 'sg556'
            case CsgoWeapon.Scout:
                return 'ssg08'
            case CsgoWeapon.Awp:
                return 'awp'
            case CsgoWeapon.Scar20:
                return 'scar20'
            case CsgoWeapon.G3sg1:
                return 'g3sg1'
            case CsgoWeapon.Nova:
                return 'nova'
            case CsgoWeapon.Xm1014:
                return 'xm1014'
            case CsgoWeapon.Mag7:
                return 'mag7'
            case CsgoWeapon.SawedOff:
                return 'sawedoff'
            case CsgoWeapon.M249:
                return 'm249'
            case CsgoWeapon.Negev:
                return 'negev'
            case CsgoWeapon.He:
                return 'hegrenade'
            case CsgoWeapon.Flashbang:
                return 'flashbang'
            case CsgoWeapon.Smoke:
                return 'smokegrenade'
            case CsgoWeapon.Decoy:
                return 'decoy'
            case CsgoWeapon.Molotov:
                return 'molotov'
            case CsgoWeapon.Incendiary:
                return 'incgrenade'
            default:
                return ''
        }
        return ''
    }

    get src(): string {
        return `assets/csgo/weapons/${this.weaponIcon}.svg`
    }

    get isValid(): boolean {
        return this.weapon !== CsgoWeapon.Unknown
    }
}

</script>