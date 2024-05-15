// #pragma once
// #include "ll/api/Logger.h"
// #include "nlohmann/json.hpp"
// #include "string"
// #include <ll/api/i18n/I18n.h>
// #include <stdexcept>
// #include <thread>


// #include "Config/Config.h"
// #include "Entry/Entry.h"
// #include "PermissionCore/Group.h"
// #include "PermissionCore/PermissionCore.h"
// #include "PermissionCore/PermissionManager.h"
// #include "PermissionCore/PermissionRegister.h"
// #include "ServerLog.h"


// namespace pmc::backend {

// using string = std::string;
// using ll::i18n_literals::operator""_tr;
// using json = nlohmann::json;

// void startServer();

// using fn = std::function<void(const httplib::Request&, httplib::Response&)>;

// #ifdef DEBUG
// #define DebugPrintRequest(req) \
//     pmc::entry::getInstance().getSelf().getLogger().warn( \
//         "[Request]: ", \
//         ServerManager::getInstance().headersToString(req.headers) \
//     );
// #else
// #define DebugPrintRequest(req) ;
// #endif


// #define AutoHandler(fn)                                                                                                \
//     [](const httplib::Request& _req, httplib::Response& _res) {                                                        \
//         DebugPrintRequest(_req);                                                                                       \
//         pmc::entry::getInstance().getSelf().getLogger().warn("[" + _req.method + "] " + _req.path);                    \
//         auto&        _manager = ServerManager::getInstance();                                                          \
//         const string _reqTime = _manager.getCurrentTimeString();                                                       \
//         try {                                                                                                          \
//             auto _token = _req.get_header_value("Authorization");                                                      \
//             if (_token.empty()) {                                                                                      \
//                 _res.status = httplib::StatusCode::Unauthorized_401;                                                   \
//                 _manager.writeHttpRequestLog(_req, _res, _reqTime);                                                    \
//                 return;                                                                                                \
//             }                                                                                                          \
//             if (_token != pmc::config::cfg.network.token) {                                                            \
//                 _res.status = httplib::StatusCode::Forbidden_403;                                                      \
//                 _manager.writeHttpRequestLog(_req, _res, _reqTime);                                                    \
//                 return;                                                                                                \
//             }                                                                                                          \
//             fn(_req, _res);                                                                                            \
//             _manager.writeHttpRequestLog(_req, _res, _reqTime);                                                        \
//         } catch (...) {                                                                                                \
//             _res.status = httplib::StatusCode::InternalServerError_500;                                                \
//             _manager.writeHttpRequestLog(_req, _res, _reqTime);                                                        \
//             throw;                                                                                                     \
//         }                                                                                                              \
//     }


// } // namespace pmc::backend