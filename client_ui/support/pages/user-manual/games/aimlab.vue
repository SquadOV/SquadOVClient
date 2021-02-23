<template>
    <div>
        <h1>Common</h1>
        <v-divider></v-divider>

        <div class="text-body-1">
            <div>
                <h2 id="history">Match History</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/aimlab/aimlab_history.jpg"
                    alt="Aim Lab game log."
                    caption="Aim Lab match history."
                />

                The Aim Lab match history page lets you view all the tasks you performed recently along with your scores.

                <ol>
                    <li>
                        <b>Task Icon.</b>
                        This icon should be identical to the one used in Aim Lab.
                    </li>
                    <li>
                        <b>Date Time.</b>
                        When you performed this particular task.
                    </li>
                    <li>
                        <b>Task Name.</b>
                        The name of the task performed.
                    </li>
                    <li>
                        <b>Score.</b>
                        Your score for the task performed.
                    </li>
                    <li>
                        <b>VOD.</b>
                        The yellow VOD indicator is displayed if there is a recorded VOD that goes along with the task.
                    </li>
                </ol>
            </div>

            <div class="mt-4">
                <h2 id="details">Task Details</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/aimlab/aimlab_task_1.jpg"
                    alt="Aim Lab match details."
                    caption="Aim Lab match details."
                />

                <ol>
                    <li>
                        <b>Match Summary.</b>
                        See <NuxtLink to="#history">Match History</NuxtLink>.
                    </li>
                    <li>
                        <b>Match Sharing Button.</b>
                        See <NuxtLink to="/user-manual/games/common#share">Match Sharing</NuxtLink>.
                    </li>
                    <li>
                        <b>VOD.</b>
                        See <NuxtLink to="/user-manual/vod#player">Video Player</NuxtLink>.
                    </li>
                    <li>
                        <b>VOD Picker.</b>
                        See <NuxtLink to="/user-manual/vod#picker">VOD Picker</NuxtLink>.
                    </li>
                </ol>

                The bottom of the Aim Lab task details page are graphs that display your progress in the stat over time in terms of some of the metrics that are collected by Aim Lab.
                For all tasks, the score is tracked (Score tab).
                For certain tasks, other metrics such as a # of kills, time to kill, and accuracy are collected (Task Specific tab).

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/aimlab/aimlab_task_2.jpg"
                    alt="Aim Lab task stats."
                    caption="Aim Lab task stats."
                />
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'

@Component
export default class GamesAimLab extends Vue {

}

</script>