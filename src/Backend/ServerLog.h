// #pragma once
// #include "Time.h"
// #include <memory>


// namespace pmc::backend {

// using string = std::string;

// class ServerLog : public Time {
// private:
//     std::mutex               logMutex;
//     std::ofstream            logFile;
//     std::thread              logThread;
//     bool                     stopLogging = false;
//     std::vector<std::string> logQueue;

//     void logWorker() {
//         while (!stopLogging) {
//             std::this_thread::sleep_for(std::chrono::seconds(1)); // Check the queue every second
//             flushLogs();
//         }
//         flushLogs(); // Ensure all logs are written before shutdown
//     }

//     void flushLogs() {
//         std::lock_guard<std::mutex> lock(logMutex);
//         if (!logFile.is_open()) {
//             openLogFile();
//         }
//         for (const auto& entry : logQueue) {
//             logFile << entry << std::endl;
//         }
//         logQueue.clear();
//         logFile.flush();
//     }

//     void openLogFile() {
//         std::string filename = "Network-" + getCurrentTimeString() + ".csv";
//         std::string filepath = "./logs/PermissionCore/" + filename;
//         logFile.open(filepath, std::ios::out | std::ios::app);
//         if (logFile.is_open() && logFile.tellp() == 0) {
//             // Write header if file is new
//             logFile << "请求时间,响应时间,来源IP,请求方法,请求路径,用户代理,请求头,状态码,密钥" << std::endl;
//         }
//     }

//     void writeLine(const std::string& entry) {
//         std::lock_guard<std::mutex> lock(logMutex);
//         logQueue.push_back(entry);
//     }

// public:
//     ServerLog() { logThread = std::thread(&ServerLog::logWorker, this); }

//     ~ServerLog() {
//         stopLogging = true;
//         if (logThread.joinable()) {
//             logThread.join();
//         }
//         if (logFile.is_open()) {
//             logFile.close();
//         }
//     }

//     string headersToString(const httplib::Headers& headers) {
//         std::stringstream ss;
//         for (const auto& header : headers) {
//             ss << "\"" << header.first << "\": \"" << header.second << "\", ";
//         }
//         string result = ss.str();
//         if (!result.empty()) {
//             result.pop_back(); // 移除最后一个逗号
//             result.pop_back(); // 移除最后一个空格
//         }
//         return result;
//     }

//     void writeHttpRequestLog(const httplib::Request& req, const httplib::Response& res, const string reqTime) {
//         std::stringstream logEntry;

//         string resTime = getCurrentTimeString();

//         logEntry << reqTime << ",";                                        // 请求时间
//         logEntry << resTime << ",";                                        // 响应时间
//         logEntry << req.remote_addr << ",";                                // 来源IP
//         logEntry << req.method << ",";                                     // 请求方法
//         logEntry << req.path << ",";                                       // 请求路径
//         logEntry << "\"" << req.get_header_value("User-Agent") << "\",";   // 用户代理
//         logEntry << "{" << headersToString(req.headers) << "},";           // 请求头
//         logEntry << res.status << ",";                                     // 状态码
//         logEntry << "\"" << req.get_header_value("Authorization") << "\""; // 密钥

//         writeLine(logEntry.str());
//     }
// };

// } // namespace pmc::backend