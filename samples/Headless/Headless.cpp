import NYANVulkanWrapper;

#include <iostream>
#include <vector>
#include <array>
#include <string_view>
#include <span>
#include <string>

using namespace nyan;

enum class ExitCode : int {
	Success = EXIT_SUCCESS,
	Failure = EXIT_FAILURE
};

int main(const int argc, char const* const* const argv)
{
	[[nodiscard]] ExitCode better_main(std::span<const std::string_view>);
	std::vector<std::string_view>
		args(argv, std::next(argv, static_cast<std::ptrdiff_t>(argc)));

	return static_cast<int>(better_main(args));
}


[[nodiscard]] ExitCode better_main([[maybe_unused]] std::span<const std::string_view> args) {
	auto name = "Headless";

	//Read console color
	//char title[255];
	//GetConsoleTitle(title, 255);
	//SetConsoleTitle(name);
	//Sleep(40);
	//auto hwnd = FindWindow(NULL, name);
	//SetConsoleTitle(title);
	//if (!hwnd)
	//	return ExitCode::Failure;
	//auto hdc = GetDC(hwnd);
	//if (!hdc)
	//	return ExitCode::Failure;


	//auto begin = "\033[38;5;";
	//auto end2 = "\033[0m";

	//std::vector < std::array<uint8_t, 3>> colors;

	//for (uint32_t i = 0; i < 255; ++i) {
	//	//auto begin2 = "\033[38;5;216m";
	//	std::cout << begin << std::to_string(i) << 'm' << (char)254u << end2 << std::endl;

	//	Sleep(40);

	//	auto pixel = GetPixel(hdc, 5, 5);
	//	std::cout << "\x1B[2J\x1B[H";
	//	std::cout << std::flush;
	//	colors.push_back({ GetRValue(pixel), GetGValue(pixel), GetBValue(pixel) });
	//}

	//for(auto color: colors)
	//	std::cout << std::format("{{{}, {}, {} }},\n", color[0], color[1], color[2]);

	return ExitCode::Success;
}