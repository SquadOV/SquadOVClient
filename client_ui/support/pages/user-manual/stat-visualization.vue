<template>
    <div>
        <h1>Stat Visualization</h1>
        <v-divider></v-divider>

        <div class="text-body-1">
            <div>
                <h2 id="overview">Overview</h2>

                The stat visualization screen allows you to visualize your stats across the games supported by SquadOV.
                Note that currently only Aim Lab stats are supported.
                To access the stat visualization screen, go to the <NuxtLink to="/user-manual/dashboard#appnav">App Navigation bar</NuxtLink>, select the Performance drop down, and click on Visualization.

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/stats/stat_viz_start.jpg"
                    alt="Add stats button."
                    caption="Click here to add stats to the visualizer."
                />

                To get started, click on the blue plus button.

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/stats/stat_select.jpg"
                    alt="Stat selection dialog."
                    caption="Select the stats you want to visualize."
                />

                In the dialog window that pops up, select the stats you wish to visualize and click “Add”.
                Finally, you will see a graph pop up with a line graph displaying the select stats.

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/stats/stat_graph.jpg"
                    alt="Stat visualization graph."
                    caption="Oooh, ahhh. Pretty graphs."
                />

                <ol>
                    <li>
                        <b>Selected Stats.</b>
                        These are the stats you selected to visualize along with controls on how to visualize the data.
                        The controls are game/stat-specific.
                        The red delete button removes the stat from the visualization.
                    </li>
                    <li>
                        <b>Stat Legend.</b>
                        Displays the name of the stat along with the color used to display the stat on the graph.
                        You can click on the stat in the legend to toggle whether it’s displayed/hidden.
                    </li>
                    <li>
                        <b>Timeline.</b>
                        Drag the white handlebars to control the range of the X-axis.
                    </li>
                    <li>
                        <b>Graph.</b>
                        A smoothed line graph of the stats in question.
                    </li>
                </ol>
            </div>

            <div class="mt-4">
                <h2 id="aimlab">Aim Lab Stats</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/stats/aimlab_stats.jpg"
                    alt="Aim Lab stat options."
                    caption="Aim Lab stat options."
                />

                There are three options you have when configuring how Aim Lab stats are displayed:

                <ul>
                    <li>
                        <b>Group.</b>
                        Controls what to use to aggregate data into a single visualized data point.
                        For example, if group is set to “Date” and function is set to “Average”, all Aim Lab tasks played on the same date will be averaged together for display.
                    </li>
                    <li>
                        <b>Function.</b>
                        Function used to aggregate data.
                        Can be average (statistical mean), min (lowest numerical value), or max (highest numerical value).
                    </li>
                    <li>
                        <b>Sort. </b>
                        Whether to sort the group value (e.g. the date) in ascending order or descending order.
                    </li>
                </ul>
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'

@Component
export default class StatVisualization extends Vue {

}

</script>