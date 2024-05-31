#include "APILogger.h"
#include "Date.h"
#include "Entry/Entry.h"
#include <filesystem>
#include <ll/api/i18n/I18n.h>


namespace pmc::backend {

void APILogger::checkAndUpdateCurrTime() {
    auto now  = utils::Date::now();
    mCurrTime = fmt::format(
        "{}-{}-{}",
        std::to_string(now.getYear()),
        std::to_string(now.getMonth()),
        std::to_string(now.getDay())
    );
}

fs::path APILogger::getAndCreateLogDir() {
    auto path = fs::current_path() / "logs" / "PermissionCore";
    if (!fs::exists(path)) {
        fs::create_directories(path);
    }
    return path;
}


using ll::i18n_literals::operator""_tr;
void APILogger::openFile() {
    checkAndUpdateCurrTime();
    auto path = getAndCreateLogDir() / ("Api-" + mCurrTime + ".csv");
    if (!fs::exists(path)) {
        fs::create_directories(path.parent_path());
    }
    mOpenedFile.open(path, std::ios::app);
    if (!mOpenedFile.is_open()) {
        entry::getInstance().getSelf().getLogger().error("Failed to open log file."_tr());
        return;
    }
    mOpenedFile << "time,ip,method,url,token" << std::endl;
}

void APILogger::checkFile() {
    // 初次运行，创建并打开日志文件
    if (!mOpenedFile.is_open()) {
        openFile();
    }
    // 日志文件过期，创建并打开新的日志文件
    auto   now  = utils::Date::now();
    string fNow = fmt::format("{}-{}-{}", now.getYear(), now.getMonth(), now.getDay());
    if (fNow == mCurrTime) return;
    mOpenedFile.close();
    openFile();
}

void APILogger::writeLine(const string& line) {
    checkFile();
    mOpenedFile << line << std::endl;
}

APILogger& APILogger::getInstance() {
    static APILogger instance;
    return instance;
}

void APILogger::log(const HttpContextPtr& ctx) {
    std::ostringstream oss;
    oss << utils::Date{}.toString() << ",";     // 请求时间
    oss << ctx->ip() << ",";                    // 来源IP
    oss << ctx->method() << ",";                // 请求方法
    oss << ctx->url() << ",";                   // 请求路径
    oss << ctx->header("Authorization") << ","; // 请求Token
    APILogger::getInstance().writeLine(oss.str());
}

void APILogger::log(HttpRequest* req) {
    std::ostringstream oss;
    oss << utils::Date{}.toString() << ",";        // 请求时间
    oss << req->host << ",";                       // 来源IP
    oss << req->method << ",";                     // 请求方法
    oss << req->url << ",";                        // 请求路径
    oss << req->GetHeader("Authorization") << ","; // 请求Token
    APILogger::getInstance().writeLine(oss.str());
}

} // namespace pmc::backend