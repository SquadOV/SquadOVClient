#pragma once

namespace service::recorder::audio {

struct AudioPacketProperties {
    size_t samplingRate = 0;

    // numSamples PER CHANNEL.
    size_t numSamples = 0;
    
    size_t numChannels = 0;
    bool isPlanar = false;
};

}