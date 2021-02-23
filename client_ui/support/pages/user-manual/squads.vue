<template>
    <div>
        <h1>Welcome to SquadOV!</h1>
        <v-divider></v-divider>

        <div class="text-body-1">
            <div>
                <h2 id="create">Creating Squads</h2>

                <div class="mt-4">
                    <h3 id="option1">Option 1</h3>

                    To create a squad, open up SquadOV and click on the "New button (1).
                    You will be then prompted to type in a name for your new squad (2).
                    After you’ve come up with your name, hit the "Create" button (3). 

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/squads/create.jpg"
                        alt="SquadOV Squad Creation."
                        caption="SquadOV Squad Creation."
                    />

                    Afterwards, you will be able to see your new Squad on your "Squads" drop down menu (4).

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/squads/squad_dropdown.jpg"
                        alt="Squads on the dashboard."
                        caption="Squads on the dashboard."
                    />
                </div>

                <div class="mt-4">
                    <h3>Option 2</h3>

                    You can also create a squad from the squad page by clicking on the squads button in the user profile dropdown in the app navigation bar.

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/squads/squad_nav.jpg"
                        alt="Navigation to the squads page."
                        caption="Navigation to the squads page."
                    />

                    Clicking on the "Create Squad" button brings up the same form found in <NuxtLink to="#option1">Option 1</NuxtLink>

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/squads/squad_page.jpg"
                        alt="Squad page."
                        caption="Squad page."
                    />

                    Afterwards, you will be able to see your new squad in the squad grid (may require a page refresh).
                </div>
            </div>

            <div class="mt-4">
                <h2 id="invite">Squad Invites</h2>

                <div class="mt-4">
                    <h3 id="inv-option1">Option 1</h3>

                    To invite a user to your Squad, click on the invite button (1) after selecting the squad on the squad dropdown. 

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/squads/dashboard_invite.jpg"
                        alt="Invite squad members from the dashboard."
                        caption="Invite squad members from the dashboard."
                    />

                    You will then be prompted to enter their username(s) or email address(es) (2).

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/squads/invite_form.jpg"
                        alt="Invite your friend."
                        caption="Invite your friends."
                    />
                    
                    After sending the invite (3), the invited user will get a notification (4) which will be represented as a red notification badge in the user profile dropdown in the app navigation bar.
                    The squad page (can be reached by clicking on the squad button in the user profile dropdown) has a tab for invites.

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/squads/invite_notification.jpg"
                        alt="Squad invite notifications."
                        caption="Squad invite notifications."
                    />

                    The invited user can then accept the invite by pressing the check button (5) or reject the invite by pressing the "x" button (6) on the squad page invites tab.
                    Invited users will also receive an email that gives them the option to accept or decline the invite from the web browser.
                    Users who are not already registered will be prompted to register first; they may register using a different email from the email you sent the email to.

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/squads/inviteemail.jpg"
                        alt="SquadOV squad invite email."
                        caption="SquadOV squad invite email."
                    />
                </div>

                <div class="mt-4">
                    <h3 id="inv-option2">Option 2</h3>

                    You can also create a squad from the squad page by clicking on the squads button in the user profile dropdown in the app navigation bar.

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/squads/squad_nav.jpg"
                        alt="Navigation to the squads page."
                        caption="Navigation to the squads page."
                    />

                    Afterwards, click on the specific squad you wish to invite your friends to to reach the squad management page.

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/squads/singlesquad.jpg"
                        alt="Squad management page."
                        caption="Squad management page."
                    />

                    Click on the blue invite button to open up the same invite form as in <NuxtLink to="#inv-option1">Option 1</NuxtLink>.
                </div>
            </div>

            <div class="mt-4">
                <h2 id="manage">Managing Squads</h2>

                To manage one of the squads you own, click on the Squad drop down menu and select the Squad you want to manage (1). 

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/squads/squad_dropdown.jpg"
                    alt="Squads on the dashboard."
                    caption="Squads on the dashboard."
                />

                Then click on the manage button (2). This will navigate you to the squad management screen. 

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/squads/managebutton.jpg"
                    alt="Squad management button."
                    caption="Squad management button."
                />

                Here you can rename your Squad (3) by pressing the pencil button and kick users by pressing the ‘x’ button next to their name (4).

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/squads/managepage.jpg"
                    alt="Squad management page."
                    caption="Squad management page."
                />

                You are also able to look at the status of your pending invites by clicking on the "Invites" tab (5).
                You can also rescind a pending invite by hitting the ‘x’ button next to their name in the invites tab (6).
                You can also delete the Squad by hitting the trash can button on the top right (7).

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/squads/managepage2.jpg"
                    alt="Squad management page - invites."
                    caption="Squad management page - invites."
                />
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'

@Component
export default class Squads extends Vue {

}

</script>