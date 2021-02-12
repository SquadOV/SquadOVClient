#include "shared/aimlab/aimlab.h"
#include "shared/time.h"

namespace shared::aimlab {
    
nlohmann::json TaskData::toJson() const {
    nlohmann::json body = {
        { "id", taskId },
        { "klutchId", klutchId },
        { "taskName", taskName },
        { "mode", mode },
        { "score", score },
        { "version", version },
        { "createDate", shared::timeToIso(createDate) },
        { "rawData", rawData }
    };
    return body;
}

}