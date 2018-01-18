#include "precompiles.h"
#include "all.h"

using namespace arma;

extern "C" {
#include "hlslcc.h"
}

shader_file::shader_file()
{
}

shader_file::shader_file(const shader_file &&right)
	: _unknown_file_pairs(std::move(right._unknown_file_pairs)), _entries(std::move(right._entries))
{
}

shader_file &shader_file::operator=(const shader_file &&right)
{
	_unknown_file_pairs = std::move(right._unknown_file_pairs);
	_entries = std::move(right._entries);

	return *this;
}

std::vector<uint8_t> shader_file::read_shader_block(arma::io::reader &reader, uint32_t expected_header, uint32_t expected_footer)
{
	uint32_t block_header, block_size, block_footer;

	reader >> block_header;

	if (block_header != expected_header)
	{
		throw format_exception("An expected internal shader boundary was not found.");
	}

	reader >> block_size;

	if (block_size > reader.remaining())
	{
		throw format_exception("The size of a shader block exceeds the remaining length of the file.");
	}

	std::vector<uint8_t> block;
	block.resize(block_size);

	reader.read_raw(block.data(), block.size());

	reader >> block_footer;

	if (block_footer != expected_footer)
	{
		throw format_exception("An expected internal shader boundary was not found.");
	}

	return std::move(block);
}

shader_file_entry shader_file::read_file_entry(arma::io::reader &reader)
{
	shader_file_entry file_entry;

	reader
		>> io::zero_terminated_string(file_entry.compiler_name)
		>> io::zero_terminated_string(file_entry.shader_name)
		>> io::zero_terminated_string(file_entry.shader_version)
		>> io::zero_terminated_string(file_entry.unknown_hex);

	const uint32_t expected_shader_magic = 0x11222211;

	uint32_t shader_magic;

	reader >> shader_magic;

	if (shader_magic != expected_shader_magic) throw format_exception(
		"An expected internal magic number was not found.");

	uint32_t entry_kind;

	reader >> entry_kind;

	const uint32_t regular_shader_entry_kind = 0x00aaaa00;
	const uint32_t regular_and_stripped_shader_entry_kind = 0x00bbbb00;

	const uint32_t regular_shader_header = 0x00cccc00;
	const uint32_t regular_shader_footer = 0x00dddd00;

	const uint32_t stripped_shader_header = 0x00eeee00;
	const uint32_t stripped_shader_footer = 0x00ffff00;

	file_entry.regular_shader = read_shader_block(reader, regular_shader_header, regular_shader_footer);

	uint32_t unknown_shader_uint32;

	reader >> unknown_shader_uint32;

	if (entry_kind == regular_and_stripped_shader_entry_kind)
	{
		file_entry.stripped_shader = read_shader_block(reader, stripped_shader_header, stripped_shader_footer);
	}

	return std::move(file_entry);
}

shader_file shader_file::read_file(arma::io::reader &reader)
{
	shader_file file;

	const std::string expected_header_magic("BIShaderCache-id: ");
	const std::string expected_header_version("0007");

	std::string header_magic;
	std::string header_version;

	reader
		>> io::sized_string(header_magic, expected_header_magic.size())
		>> io::sized_string(header_version, expected_header_version.size() + 1);

	if (header_magic != expected_header_magic) throw format_exception(
		"The file does not appear to be a valid shader library.");

	if (header_magic != expected_header_magic) throw format_exception(
		"This application does not support the version of the specified file.");

	const uint32_t expected_after_shader_headers = 11;

	while (true)
	{
		uint32_t shader_size;
		uint32_t shader_other;

		reader >> shader_size;

		if (shader_size == expected_after_shader_headers) break;

		reader >> shader_other;

		file._unknown_file_pairs.emplace_back(shader_size, shader_other);
	}

	const uint32_t expected_unknown_magic = 0xaa55aa55;

	uint32_t unknown_seven;
	uint32_t unknown_magic;
	uint32_t unknown_zero;

	reader >> unknown_seven >> unknown_magic >> unknown_zero;

	if (unknown_seven != 7 ||
		unknown_magic != expected_unknown_magic ||
		unknown_zero != 0) throw format_exception(
		"An unexpected magic was found in the shader library header.");

	uint32_t unknown_other;

	reader >> unknown_other;

	while (reader.remaining() > 0)
	{
		file._entries.push_back(read_file_entry(reader));
	}

	return std::move(file);
}

void shader_file::dump_shader_to_hlsl_assembly(const std::wstring &destination_path, const shader_file_entry &shader)
{
	ID3D10Blob *blob;

	HRESULT result = D3DDisassemble(
		shader.regular_shader.data(), shader.regular_shader.size(),
		D3D_DISASM_ENABLE_DEFAULT_VALUE_PRINTS | D3D_DISASM_ENABLE_INSTRUCTION_NUMBERING,
		"", &blob);

	auto deleter = [](ID3D10Blob *blob) { blob->Release(); };

	std::unique_ptr<ID3D10Blob, decltype(deleter)> blob_deleter(blob, deleter);

	if (result != S_OK) throw format_exception("Unable to disassemble one of the shaders.");

	std::ofstream disassembly_file;
	disassembly_file.open(destination_path);
	disassembly_file.write(static_cast<const char *>(blob->GetBufferPointer()), blob->GetBufferSize());

	if (disassembly_file.bad()) throw application_exception("Unable to write a shader dissassembly file.");
}

void shader_file::dump_shader_to_glsl(const std::wstring &destination_path, const shader_file_entry &entry)
{
	GlExtensions extensions;

	extensions.ARB_explicit_attrib_location = 1;
	extensions.ARB_explicit_uniform_location = 1;
	extensions.ARB_shading_language_420pack = 1;

	GLSLShader result;

	auto result_code = TranslateHLSLFromMem(
		reinterpret_cast<const char *>(entry.regular_shader.data()),
		0, // flags
		LANG_440,
		&extensions,
		nullptr,
		&result);

	std::ofstream disassembly_file;
	disassembly_file.open(destination_path);
	disassembly_file.write(result.sourceCode, strlen(result.sourceCode));

	if (disassembly_file.bad()) throw application_exception("Unable to write a shader dissassembly file.");
}

void shader_file::dump_shaders(const std::wstring &directory)
{
	for (const auto &shader : _entries)
	{
		std::wstring disassembly_prefix(directory);

		if (disassembly_prefix.size() > 0 && disassembly_prefix.back() != L'\\')
		{
			disassembly_prefix += L'\\';
		}

		disassembly_prefix += std::wstring(shader.shader_name.begin(), shader.shader_name.end());

		dump_shader_to_hlsl_assembly(disassembly_prefix + L".txt", shader);
		dump_shader_to_glsl(disassembly_prefix + L".glsl", shader);
	}
}
