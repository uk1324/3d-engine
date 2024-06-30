#include <iostream>
#include <filesystem>

using namespace std;
using namespace filesystem;

const path outputPath = "./generated/build/";

int main(int argc, char** argv) {
	cout << "working directory = " << current_path().string() << '\n';

	if (argc != 2) {
		cerr << "wrong number of arguments " << argc << '\n';
		return EXIT_FAILURE;
	}
	const string_view executablePathString(argv[1]);
	cout << "executable path = " << executablePathString << '\n';

	const path executablePath(executablePathString);

	try {
		const auto gameOutputPath = outputPath / "game";
		create_directories(gameOutputPath);
		#ifdef WIN32
		copy_file("./freetype.dll", gameOutputPath / "freetype.dll", copy_options::overwrite_existing);
		copy_file("./OpenAL32.dll", gameOutputPath / "OpenAL32.dll", copy_options::overwrite_existing);
		#endif
		create_directories(gameOutputPath / "assets");
		copy("./platformer/assets", gameOutputPath / "assets", copy_options::recursive | copy_options::overwrite_existing);
		copy("./assets/fonts/RobotoMono-Regular.ttf", gameOutputPath / "assets/fonts/RobotoMono-Regular.ttf", copy_options::recursive | copy_options::overwrite_existing);
		copy_file(executablePath, gameOutputPath / ("axometric" + executablePath.extension().string()), copy_options::overwrite_existing);
	} catch (const filesystem_error& e) {
		cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}
}