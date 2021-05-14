#include <boost/asio.hpp>
#include "shared/time/ntp_client.h"
#include "shared/log/log.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/endian/buffers.hpp>
#include <chrono>
#include <memory>
#include <limits>
#include <string>
#include <vector>
#include <boost/array.hpp>

using boost::asio::ip::udp;
using namespace boost::endian;

namespace shared::time {
namespace {

constexpr int32_t NTP_TICK_INTERVAL_SECONDS = 1024;
constexpr int64_t NTP_TIMESTAMP_DELTA = 2208988800;
constexpr int64_t NTP_ERROR_MS_THRESHOLD = 43200000;

struct NTPTimestamp {
    NTPTimestamp() {
        seconds = 0;
        fractional = 0;
    }
    big_uint32_buf_t seconds;
    big_uint32_buf_t fractional;

    shared::TimePoint tm() const;
};

shared::TimePoint NTPTimestamp::tm() const {
    // Convert the seconds to unix time.
    shared::TimePoint base = shared::unixMsToTime(static_cast<int64_t>(seconds.value() - NTP_TIMESTAMP_DELTA) * 1000);

    // Add the fractional part as number of nanoseconds.
    const auto frac = static_cast<double>(fractional.value()) / std::numeric_limits<uint32_t>::max();
    const auto newBase = base + std::chrono::nanoseconds(static_cast<int64_t>(frac * 1e9));
    return shared::convertTime(newBase);
}

struct NTPPacket {
    NTPPacket() {
        liVnMode = 0;
        stratum = 0;
        poll = 0;
        precision = 0;
        rootDelay = 0;
        rootDispersion = 0;
        referenceId = 0;
    }

    big_uint8_buf_t liVnMode;
    big_uint8_buf_t stratum;
    big_uint8_buf_t poll;
    big_int8_buf_t precision;
    big_int32_buf_t rootDelay;
    big_int32_buf_t rootDispersion;
    big_int32_buf_t referenceId;

    // Timestamps
    NTPTimestamp refTm;
    NTPTimestamp originTm;
    NTPTimestamp rxTm;
    NTPTimestamp txTm;
};

const std::vector<std::string> NTP_SERVERS = {
    // We use servers that DO NOT LEAP SMEAR.
    "0.pool.ntp.org",
    "1.pool.ntp.org",
    "2.pool.ntp.org",
    "3.pool.ntp.org",
    "time.cloudflare.com"
};

}

NTPClient* NTPClient::singleton() {
    static auto global = std::make_unique<NTPClient>();
    return global.get();    
}

NTPClient::~NTPClient() {
    _running = false;
    if (_tickThread.joinable()) {
        _tickThread.join();
    }
}

void NTPClient::initialize() {
    LOG_INFO("Initializing NTP client..." << std::endl);
    _running = true;
    // Start a thread that refreshes the offset every once in awhile.
    _tickThread = std::thread([this](){
        const auto interval = std::chrono::seconds(NTP_TICK_INTERVAL_SECONDS);
        const auto step = std::chrono::seconds(1);
        auto elapsed = std::chrono::seconds(NTP_TICK_INTERVAL_SECONDS);

        while (_running) {
            // Run the tick in a thread since we don't want it to block the entire program if we aren't able to
            // get a response from an NTP server in a timely fashion.
            if (elapsed >= interval) {
                tick();
                elapsed = std::chrono::seconds(0);
            }

            std::this_thread::sleep_for(step);
            elapsed += step;
        }
    });
}

void NTPClient::enable(bool enabled, bool doTick) {
    _enabled = enabled;
    if (doTick) {
        tick();
    }
}

void NTPClient::tick() {
    LOG_INFO("...Performing NTP tick." << std::endl);
    int64_t newOffset = 0;

    if (_enabled) {
        std::vector<int64_t> offsets;
        offsets.reserve(NTP_SERVERS.size());
        for (auto i = 0; i < NTP_SERVERS.size(); ++i) {
            const auto os = offsetToServer(NTP_SERVERS[i]);

            // Do a reasonableness check on the new offset.
            if (os >= 0 && os <= NTP_ERROR_MS_THRESHOLD) {
                offsets.push_back(os);
            } else {
                LOG_WARNING("Computed an NTP offset that's unreasonable...ignoring it: " << os << std::endl);
            }
        }
        std::sort(offsets.begin(), offsets.end());

        // Remove the smallest and highest value and use the average of the rest.
        auto den = 0;

        // In the off chance we don't have enough elements to trim the largest and smallest values then we use everything.
        if (offsets.size() > 2) {
            for (auto i = 1; i < offsets.size() - 1; ++i) {
                newOffset += offsets[i];
                den += 1;
            }
        } else {
            for (const auto& v : offsets) {
                newOffset += v;
                den += 1;
            }
        }

        if (den > 0) {
            newOffset /= den;
        }
    }

    {
        // Set offset here to minimize the time we actually hold the mutex exclusively.
        std::lock_guard guard(_offsetMutex);
        _offsetMs = newOffset;
    }

    const auto cn = clientNow();
    const auto nw = now();
    LOG_INFO("Computed NTP offset: " << newOffset << "ms" << std::endl
        << "\tClient: " << shared::timeToStr(cn) << std::endl
        << "\tSynced: " << shared::timeToStr(nw) << std::endl);
}

shared::TimePoint NTPClient::clientNow() const {
    return std::chrono::system_clock::now();
}

int64_t NTPClient::offsetToServer(const std::string& server) const {
    LOG_INFO("Computing NTP offset to " << server << std::endl);
    boost::asio::io_context ioContext;
    boost::asio::io_service ioService;
    udp::resolver resolver(ioContext);

    NTPPacket packet;
    // li - 0, vn - 4, mode - 3.
    // 00 100 011
    packet.liVnMode = 0b00100011;

    auto buffer = boost::asio::buffer((void*)&packet, sizeof(packet));

    shared::TimePoint t0;
    shared::TimePoint t3;

    try {
        udp::endpoint endpoint = *resolver.resolve(udp::v4(), server, "ntp").begin();

        udp::socket socket(ioService);
        socket.open(udp::v4());

        t0 = clientNow();
        socket.send_to(buffer, endpoint);

        udp::endpoint senderEndpoint;
        bool received = false;

        socket.async_receive_from(buffer, senderEndpoint, [this, &t3, &received, &ioService](const boost::system::error_code& err, std::size_t bt){
            t3 = clientNow();
            received = true;
            ioService.stop();
        });

        boost::asio::deadline_timer timer(ioService);
        timer.expires_from_now(boost::posix_time::seconds(5));
        timer.async_wait([&socket](const boost::system::error_code& err){
            socket.close();
        });

        ioService.run();

        if (!received) {
            LOG_WARNING("Timeout in getting response from NTP server." << std::endl);
            return -1;
        }
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to find NTP offset to " << server << ": " << ex.what() << std::endl);
        return -1;
    }

    const auto epoch = shared::unixMsToTime(0);
    const auto t1 = packet.rxTm.tm();
    const auto t2 = packet.txTm.tm();

    if (t1 <= epoch || t2 <= epoch) {
        LOG_INFO("Detected pre-epoch times from the server...ignoring." << std::endl);
        return -1;
    }

    const auto d1 = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    const auto d2 = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t3).count();
    const auto offset = std::abs((d1 + d2) / 2);
    LOG_INFO("...Offset: " << offset << "ms" << std::endl);
    LOG_INFO("\tT0: " << shared::timeToStr(t0) << std::endl);
    LOG_INFO("\tT1: " << shared::timeToStr(t1) << std::endl);
    LOG_INFO("\tT2: " << shared::timeToStr(t2) << std::endl);
    LOG_INFO("\tT3: " << shared::timeToStr(t3) << std::endl);
    return offset;
}

shared::TimePoint NTPClient::now() const {
    return adjustTime(clientNow());
}

shared::TimePoint NTPClient::adjustTime(const shared::TimePoint& tm) const {
    std::shared_lock guard(_offsetMutex);
    return tm + std::chrono::milliseconds(_offsetMs);
}

shared::TimePoint NTPClient::revertTime(const shared::TimePoint& tm) const {
    std::shared_lock guard(_offsetMutex);
    return tm - std::chrono::milliseconds(_offsetMs);
}

}