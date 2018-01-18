#include "precompiles.h"
#include "all.h"

namespace arma {
namespace io {

reader::reader(const std::wstring &path)
{
	_stream.open(path, std::ios_base::binary | std::ios_base::in);

	if (_stream.fail()) throw format_exception("The input file could not be found");

	_stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	// Find the length:
	_stream.seekg(0, std::ios::end);
	_stream_size = _stream.tellg();
	_stream.seekg(0, std::ios::beg);
}

std::size_t reader::scan_for_terminator(uint8_t terminator)
{
	std::streamoff beginning = _stream.tellg();

	while (!_stream.eof())
	{
		char candidate;

		_stream.get(candidate);

		if (candidate == terminator) break;
	}

	std::size_t size = (std::size_t)(_stream.tellg() - beginning) - (_stream.eof() ? 0 : 1);

	_stream.seekg(beginning, std::ios::beg);

	return size;
}

std::size_t reader::scan_for_terminator(uint8_t terminator, uint32_t maximum_size)
{
	uint32_t remaining = maximum_size;

	std::streamoff beginning = _stream.tellg();

	while (!_stream.eof() && remaining > 0)
	{
		char candidate;

		_stream.get(candidate);

		if (candidate == terminator) break;

		remaining--;
	}

	std::size_t size = (std::size_t)(_stream.tellg() - beginning) - ((_stream.eof() || remaining == 0) ? 0 : 1);

	_stream.seekg(beginning, std::ios::beg);

	return size;
}

}
}