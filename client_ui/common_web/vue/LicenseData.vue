<template>
    <v-tabs>
        <v-tab>
            FFmpeg
        </v-tab>

        <v-tab-item>
            <div class="ma-4 license-div">
                <div>
                    <span class="font-weight-bold">URL: </span>
                    <a href="#">https://ffmpeg.org/</a>
                </div>
                <v-divider class="my-2"></v-divider>

                <div>
                    <span class="font-weight-bold">Source Code: </span>
                    <a href="#">https://gitlab.com/squadov/ffmpeg</a>
                </div>
                <v-divider class="my-2"></v-divider>

                <div>
                    <div class="font-weight-bold">
                        Compilation Instructions
                    </div>

                    <div>
                        <ol>
                            <li>Download and compile OpenH264 from <a href="#">https://github.com/cisco/openh264</a>.</li>
                            <li>Download the AMF headers from <a href="#">https://github.com/GPUOpen-LibrariesAndSDKs/AMF</a>.</li>
                            <li>Download the Nvidia codec headers from <a href="#">https://github.com/FFmpeg/nv-codec-headers</a>.</li>
                            <li>
                                <div>Compile FFmpeg using MSYS2 using the following configure command:</div>
                                <div>
                                    <pre>
./configure --prefix=../../prebuilt/ffmpeg \
    --enable-shared \
    --disable-doc \
    --enable-libopenh264 \
    --enable-amf \
    --enable-nvenc \
    --toolchain=msvc \
    --target-os=win64 \
    --arch=x86_64 \
    --env="PKG_CONFIG_PATH=../../prebuilt/openh264/lib/pkgconfig"
                                    </pre>
                                </div>
                                <div>
                                    Any pre-compiled FFmpeg 4.x libraries built with at least one of OpenH264, AMF, and NVENC should be usable with SquadOV.
                                </div>
                            </li>
                        </ol>
                    </div>
                </div>
                <v-divider class="my-2"></v-divider>

                <div>
                    <div class="font-weight-bold">
                        Installation Instructions
                    </div>

                    <div>
                        <ol>
                            <li>Navigate to the SquadOV client service installation directory (usually: <code>%LOCALAPPDATA%\Programs\squadov_client_ui\resources\service</code>).</li>
                            <li>Replace <code>avcodec-58.dll</code>, <code>avdevice-58.dll</code>, <code>avfilter-7.dll</code>, <code>avformat-7.dll</code>, and  <code>avutil-58.dll</code> with your replacement binaries.</li>
                        </ol>
                    </div>
                </div>
            </div>
        </v-tab-item>
    </v-tabs>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'

@Component
export default class LicenseData extends Vue {

}

</script>

<style scoped>

.license-div {
    max-height: 400px;
    height: 400px;
    overflow: auto;
}

</style>