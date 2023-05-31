#pragma once

class GenericServerIface {
public:
    virtual ~GenericServerIface() = default;

    virtual void run() = 0;

    virtual void stop() = 0;
};