#pragma once

struct shader_file_entry
{
	std::string compiler_name;
	std::string shader_name;
	std::string shader_version;
	std::string unknown_hex;

	std::vector<uint8_t> regular_shader;
	std::vector<uint8_t> stripped_shader;
};
