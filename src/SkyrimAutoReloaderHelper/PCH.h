#pragma once

#pragma warning(push)

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#ifndef NDEBUG
#	include <spdlog/sinks/msvc_sink.h>
#endif

using namespace std::literals;

#pragma warning(pop)
