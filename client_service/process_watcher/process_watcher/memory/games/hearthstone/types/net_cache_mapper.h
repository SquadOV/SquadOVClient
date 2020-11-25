#pragma once

#include "process_watcher/memory/mono/mono_image_mapper.h"
#include "process_watcher/memory/mono/mono_object_mapper.h"
#include "process_watcher/memory/mono/types/map_mapper.h"
#include <memory>
#include <unordered_map>

namespace process_watcher::memory::games::hearthstone::types {

class NetCacheMapper {
public:
    explicit NetCacheMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);
    static const process_watcher::memory::mono::MonoClassMapper* klass(const process_watcher::memory::mono::MonoImageMapper& image);

    using InternalCacheType = process_watcher::memory::mono::types::MapMapper<process_watcher::memory::mono::MonoObjectMapperSPtr, process_watcher::memory::mono::MonoObjectMapperSPtr>;
    InternalCacheType cache() const;

    class BaconRatingInfoMapper {
    public:
        explicit BaconRatingInfoMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);

        int32_t rating() const;
    private:
        process_watcher::memory::mono::MonoObjectMapperSPtr _object;
    };
    using BaconRatingInfoMapperSPtr = std::shared_ptr<BaconRatingInfoMapper>;
    BaconRatingInfoMapperSPtr getBaconRatingInfo() const;

    class PvpDrStatsInfoMapper {
    public:
        explicit PvpDrStatsInfoMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object);

        int32_t rating() const;
        int32_t paidRating() const;
    private:
        process_watcher::memory::mono::MonoObjectMapperSPtr _object;
    };
    using PvpDrStatsInfoMapperSPtr = std::shared_ptr<PvpDrStatsInfoMapper>;
    PvpDrStatsInfoMapperSPtr getPvpDrStatsInfo() const;

private:
    process_watcher::memory::mono::MonoObjectMapperSPtr _object;
    std::unordered_map<std::string, process_watcher::memory::mono::MonoObjectMapperSPtr> _cachedObjects;
};
using NetCacheMapperSPtr = std::shared_ptr<NetCacheMapper>;

}