// #pragma once
// #include "API.h"
// #include <functional>
// #include <thread>


// namespace pmc::backend {

// This function defined in the header file.
// void startServer() {
// auto& cfg = pmc::config::cfg.network;

// #ifdef DEBUG
//     std::cout << "[DBG] Config: " << std::endl;
//     std::cout << "  Enable: " << cfg.enable << std::endl;
//     std::cout << "  Listen IP: " << cfg.listenIP << std::endl;
//     std::cout << "  Listen Port: " << cfg.listenPort << std::endl;
//     std::cout << "  Allow CORS: " << cfg.allowCORS << std::endl;
//     std::cout << "  Token: " << cfg.token << std::endl;
// #endif

// if (!cfg.enable) return;


// clang-format off
    /*
        TODO: We need find a good and header-only's C++ Http library.
        The following libraries are excluded:
            cpp-httplib
            cinatra
            drogon
    */
// clang-format on


// Create server instance
// auto& logger = pmc::entry::getInstance().getSelf().getLogger();
// drogon::app()
//     .setLogPath("./logs/PermissionCore/net")
//     .setLogLevel(trantor::Logger::LogLevel::kInfo)
//     .addListener(cfg.listenIP, cfg.listenPort)
//     .setThreadNum(4)
//     .enableRunAsDaemon();


// drogon::app().registerHandler(
//     "/pmc/validate",
//     [](const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
//         string token = req->getHeader("Authorization");
//         pmc::entry::getInstance().getSelf().getLogger().warn("Validate token: ", token);
//         callback(drogon::HttpResponse::newHttpJsonResponse("hello world"));
//     },
//     {drogon::Get}
// );


// Set server routes
// svr.set_http_handler<cinatra::GET>(
//     "/pmc/validate",
//     [](cinatra::request&, cinatra::response& res) {
//         res.set_status_and_content(cinatra::status_type::ok, "Hello, world!");
//     },
//     mCORS{},
//     mLog{},
//     mAuth{}
// );
// svr.set_http_handler("/pmc/list/all/core", handler_ListCore);
// svr.set_http_handler("/pmc/list/all/plugin", handler_ListPlugn));
// svr.set_http_handler("/pmc/list/all/perm", handler_ListPerm);


// Start server
// logger.info("Try Start server on {}:{}."_tr(cfg.listenIP, cfg.listenPort));
// std::thread([&svr]() {
//     auto err = svr.sync_start();
// pmc::entry::getInstance().getSelf().getLogger().error("Fail in \"sync_start\", code: ",
// static_cast<int>(err));
//     printf("Fail in \"sync_start\", code: %d\n", static_cast<int>(err));
// }).detach();
// svr.async_start();
// drogon::app().run();
// }

// } // namespace pmc::backend