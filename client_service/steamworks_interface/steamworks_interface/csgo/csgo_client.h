#pragma once

#include <vector>

namespace steamworks_interface::csgo {

class CsgoClient {
public:
    void waitForConnect();
    void getRecentCsgoMatchList();
};

}