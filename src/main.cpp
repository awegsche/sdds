#include "sddslib.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

int main(int argc, char** argv) {

	auto console = spdlog::stdout_color_mt("console");

	spdlog::set_pattern("%^%6l%$ | %v");
	spdlog::set_level(spdlog::level::trace);
	SPDLOG_INFO("Hello CMAKE");
	SPDLOG_ERROR("Error");
	SPDLOG_DEBUG("debug");

	sdds::SddsFile file("K:/CERN/tfs/sdds1.sdds");

	return 0;
}