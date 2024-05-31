#pragma once
#include <string>
using string = std::string;

struct mConfig {
    int version = 2;
    struct mNetwork {
        bool           Enable    = true;
        unsigned short Port      = 11451;
        string         Token     = "default_token";
        bool           AllowCORS = true;
    } Network;
};

namespace pmc::config {

extern mConfig cfg;

void loadConfig();

} // namespace pmc::config