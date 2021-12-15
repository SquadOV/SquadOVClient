#pragma once

namespace service::recorder {

class ProcessRecordInterface {
public:
    virtual ~ProcessRecordInterface() {}
    virtual void forceStopRecording() = 0;
};

}