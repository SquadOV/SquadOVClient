#pragma once

namespace shared::time {

class NTPClient {
public:
    static NTPClient* singleton();

    void initialize();
};

}