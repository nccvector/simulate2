
#ifndef LOGGING_H
#define LOGGING_H


#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/spdlog.h>


// Use default logger for now...
#define DEBUG( ... ) SPDLOG_LOGGER_DEBUG( spdlog::default_logger(), __VA_ARGS__ )
#define INFO( ... )  SPDLOG_LOGGER_INFO( spdlog::default_logger(), __VA_ARGS__ )
#define WARN( ... )  SPDLOG_LOGGER_INFO( spdlog::default_logger(), __VA_ARGS__ )
#define ERROR( ... ) SPDLOG_LOGGER_ERROR( spdlog::default_logger(), __VA_ARGS__ )
// Now you may set the logging level at your application entry point...


#endif // LOGGING_H
