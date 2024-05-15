#pragma once
#include <string>
using string = std::string;

struct mConfig {
    int version = 2;
    struct mNetwork {
        bool           enable     = false;
        string         listenIP   = "0.0.0.0";
        unsigned short listenPort = 11451;
        string         token      = "88888888";
        bool           allowCORS  = false;
    } network;
};

namespace pmc::config {

extern mConfig cfg;

void loadConfig();

} // namespace pmc::config