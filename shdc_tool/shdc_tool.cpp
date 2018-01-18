#include "precompiles.h"
#include "all.h"

using namespace arma;

int _tmain(int argc, _TCHAR *argv[])
{
	try
	{
		std::vector<std::wstring> arguments;
		
		std::copy(argv + 1, argv + argc, std::back_inserter(arguments));

		if (arguments.size() != 2)
		{
			throw application_exception("Usage: shdc_tool.exe <shdc file> <output directory>");
		}

		std::wstring shdc_path = arguments[0];
		std::wstring output_directory = arguments[1];

		if (!filesystem::is_file(shdc_path)) throw application_exception(
			"The specified shdc file does not exist.");

		if (!filesystem::is_directory(output_directory)) throw application_exception(
			"The specified shdc directory does not exist.");

		io::reader reader(shdc_path);

		auto file = shader_file::read_file(reader);

		file.dump_shaders(output_directory);
	}
	catch (const format_exception &e)
	{
		std::cerr << e.message() << std::endl;
	}
	catch (const application_exception &e)
	{
		std::cerr << e.message() << std::endl;
	}

	return 0;
}
