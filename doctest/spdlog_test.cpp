/*
 * Created by HelliWrold1 on 2023/3/27 15:13.
 */

#include "doctest.h"
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"


auto console = spdlog::stdout_color_mt( "logger" );

TEST_CASE("test spdlog")
{
    char ss[] = "hi";
    auto logger = spdlog::get("logger");
//    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%l][%s: %!:%#]: %v");
    logger->set_level(spdlog::level::debug);
    spdlog::set_level(spdlog::level::debug);
//    console->debug("[DB]: auto increment id:{}", 21);
    SPDLOG_LOGGER_DEBUG(logger, "test3 {}" ,ss);//会输出文件名和行号
    SPDLOG_DEBUG("test56 {}",ss);
    SPDLOG_INFO("test88 {}",ss);
}