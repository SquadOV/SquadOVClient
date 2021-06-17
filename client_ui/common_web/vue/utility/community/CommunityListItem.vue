<template>
    <v-list-item :to="to">
        <v-list-item-avatar
            rounded
            width="36px"
            height="36px"
            :color="backgroundColor"
        >
            <div class="font-weight-bold text-h6" :style="avatarTextStyle">
                {{ firstChar }}
            </div>
        </v-list-item-avatar>

        <v-list-item-content>
            <v-list-item-title>
                {{ community.name }}
            </v-list-item-title>

            <v-list-item-subtitle>
                <span>{{ `/c/${community.slug} `}}</span>
                <span>({{ securityLevel }})</span>

                <template v-if="community.requiresSubscription && community.securityLevel == CommunitySecurityLevel.Private">
                    <v-icon x-small>
                        mdi-currency-usd
                    </v-icon>

                    <v-icon x-small v-if="community.allowTwitchSub">
                        mdi-twitch
                    </v-icon>
                </template>
            </v-list-item-subtitle>
        </v-list-item-content>
    </v-list-item>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { SquadOvCommunity, communitySecurityLevelToString, CommunitySecurityLevel } from '@client/js/squadov/community'
import { colorToCssString, generateUniqueColor } from '@client/js/color'
import { CommunityPageId } from '@client/js/pages'

@Component
export default class CommunityListItem extends Vue {
    CommunitySecurityLevel = CommunitySecurityLevel

    @Prop({required: true})
    community!: SquadOvCommunity

    @Prop({type: Boolean, default: false})
    noLink!: boolean

    get backgroundColor(): string {
        return colorToCssString(generateUniqueColor(this.firstChar.charCodeAt(0)))
    }

    get firstChar(): string {
        return this.community.name[0].toLocaleUpperCase()
    }

    get avatarTextStyle(): any {
        return {
            'color': this.backgroundColor,
            'filter': 'invert(100%)',
            '-webkit-text-stroke': '0.2px white',
        }
    }

    get securityLevel(): string {
        return communitySecurityLevelToString(this.community.securityLevel)
    }

    get to(): any {
        if (this.noLink) {
            return undefined
        } else {
            return {
                name: CommunityPageId,
                params: {
                    communitySlug: this.community.slug,
                }
            }
        }
    }
}

</script>