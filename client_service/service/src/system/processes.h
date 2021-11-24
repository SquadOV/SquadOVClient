#pragma once

#include "process_watcher/process/process.h"
#include <vector>

namespace service::system {

std::vector<process_watcher::process::ProcessRecord> getListOfUserFacingProcesses();

}