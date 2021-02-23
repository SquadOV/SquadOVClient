<template>
    <div>
        <h1>Account</h1>
        <v-divider></v-divider>

        <div class="text-body-1">
            <div>
                <h2 id="registration">Registration</h2>

                To register for a SquadOV account, open SquadOV and hit the “Register” (1) button at the login screen.

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/account/register.jpg"
                    alt="Link to registration screen."
                    caption="You can find the link to the registration screen on the initial login screen."
                />

                Then type in a username, email and password that you will remember (2).

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/account/register_form.jpg"
                    alt="SquadOV registration form."
                    caption="Your typical registration form: username, email, and password."
                />

                Hit the register button when the fields have been filled out (3).
                An automatic verification email will be sent to the aforementioned email address. Click on the link to verify your email address (4).

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/account/verify.jpg"
                    alt="SquadOV verification email."
                    caption="Click on the link in the verification email to be able to login."
                />

                After you have completed the verification, your account is ready to be used!
            </div>

            <div class="mt-4">
                <h2 id="login" >Login</h2>

                To login to your SquadOV account, please type in your username and password (1) and hit the login button:

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/account/login.jpg"
                    alt="Login screen."
                    caption="The SquadOV login screen, pretty normal."
                />

                If you forget your password, see below for instructions on how to set a new password.
            </div>

            <div class="mt-4">
                <h2 id="forgotpw">Forgot Password</h2>

                To reset your password, please click on the “Forgot your password?” link on the SquadOV login page after entering your username or email address (1).

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/account/forgotpw.jpg"
                    alt="Forgot password link."
                    caption="Click here if you forgot your password."
                />

                An automatic email will be sent to the email address you provided to change your password.

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/account/resetpwemail.jpg"
                    alt="Reset password email."
                    caption="Click this link in your email to reset your password."
                />

                Clicking on the link (2) will allow you to type in a new password (3). You will then be able to login normally with the new password.

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/account/newpw.jpg"
                    alt="Type in a new password."
                    caption="Enter in a new password, don't forget it this time! Smiley face."
                />
            </div>

            <div class="mt-4">
                <h2 id="profile">User Profile</h2>

                To access your user profile, click on your username at the top right of your SquadOV window. A drop down menu will appear. Hit ‘Profile’ (1) to navigate to your user profile.

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/account/userprofile.jpg"
                    alt="User profile dropdown."
                    caption="Click here to view your (barebones) user profile."
                />
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'

@Component
export default class Account extends Vue {

}

</script>