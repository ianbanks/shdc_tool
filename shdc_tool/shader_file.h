#pragma once

class shader_file
{
public:
	shader_file();

	shader_file(const shader_file &) = delete;
	shader_file &operator=(const shader_file &) = delete;

	shader_file(const shader_file &&);
	shader_file &operator=(const shader_file &&);

	static shader_file shader_file::read_file(arma::io::reader &reader);

	void dump_shaders(const std::wstring &directory);

	static void dump_shader_to_hlsl_assembly(const std::wstring &destination_path, const shader_file_entry &shader);
	static void dump_shader_to_glsl(const std::wstring &destination_path, const shader_file_entry &shader);

private:
	static std::vector<uint8_t> read_shader_block(arma::io::reader &reader, uint32_t expected_header, uint32_t expected_footer);
	static shader_file_entry shader_file::read_file_entry(arma::io::reader &reader);

	// A list of unknown pairs at the start of the file; the first of the pair appears
	// to be a size for something not stored in the shader file:
	std::vector<std::pair<uint32_t, uint32_t>> _unknown_file_pairs;

	// The shaders themselves:
	std::vector<shader_file_entry> _entries;
};
