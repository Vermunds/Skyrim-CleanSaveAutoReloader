#pragma once

#pragma warning(push)

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#ifndef NDEBUG
#	include <spdlog/sinks/msvc_sink.h>
#endif

using namespace std::literals;

#pragma warning(pop)
