#include "process_watcher/memory/games/hearthstone/types/net_cache_mapper.h"

namespace process_watcher::memory::games::hearthstone::types {
namespace {

const std::string NETCACHE_FULL_NAME("NetCache");
const std::string NETCACHE_FIELD_NAME("m_netCache");
const std::string NETCACHE_BACON_RATING_CLASS("NetCacheBaconRatingInfo");
const std::string NETCACHE_PVPDR_STATS_CLASS("NetCachePVPDRStatsInfo");

const std::string BACON_RATING_FIELD("<Rating>k__BackingField");
const std::string PVPDR_RATING_FIELD("<Rating>k__BackingField");
const std::string PVPDR_PAID_RATING_FIELD("<PaidRating>k__BackingField");

}
NetCacheMapper::NetCacheMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {
    // Do an O(N) process here where we store the class name of the object as the key instead of the type
    // as I'm really not sure how to grab an object's System.Type for indexing.
    auto internalCache = cache();
    for (const auto& kvp: internalCache.values()) {
        _cachedObjects[kvp.second->klass()->fullName()] = kvp.second;
    }
}

const process_watcher::memory::mono::MonoClassMapper* NetCacheMapper::klass(const process_watcher::memory::mono::MonoImageMapper& image) {
    return image.loadClassFromFullName(NETCACHE_FULL_NAME);
}

NetCacheMapper::InternalCacheType NetCacheMapper::cache() const {
    const auto object = _object->get(NETCACHE_FIELD_NAME).get<mono::MonoObjectMapperSPtr>();
    return InternalCacheType(object);
}

NetCacheMapper::BaconRatingInfoMapperSPtr NetCacheMapper::getBaconRatingInfo() const {
    auto it = _cachedObjects.find(NETCACHE_BACON_RATING_CLASS);
    if (it == _cachedObjects.end()) {
        return nullptr;
    }
    return std::make_shared<NetCacheMapper::BaconRatingInfoMapper>(it->second);
}

NetCacheMapper::BaconRatingInfoMapper::BaconRatingInfoMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {
}

int32_t NetCacheMapper::BaconRatingInfoMapper::rating() const {
    const auto object = _object->get(BACON_RATING_FIELD);
    return object.get<int32_t>();
}

NetCacheMapper::PvpDrStatsInfoMapperSPtr NetCacheMapper::getPvpDrStatsInfo() const {
    auto it = _cachedObjects.find(NETCACHE_PVPDR_STATS_CLASS);
    if (it == _cachedObjects.end()) {
        return nullptr;
    }
    return std::make_shared<NetCacheMapper::PvpDrStatsInfoMapper>(it->second);
}

NetCacheMapper::PvpDrStatsInfoMapper::PvpDrStatsInfoMapper(const process_watcher::memory::mono::MonoObjectMapperSPtr& object):
    _object(object) {
}

int32_t NetCacheMapper::PvpDrStatsInfoMapper::rating() const {
    const auto object = _object->get(PVPDR_RATING_FIELD);
    return object.get<int32_t>();
}

int32_t NetCacheMapper::PvpDrStatsInfoMapper::paidRating() const {
    const auto object = _object->get(PVPDR_PAID_RATING_FIELD);
    return object.get<int32_t>();
}

}