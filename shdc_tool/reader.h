#pragma once

namespace arma {
namespace io {

class reader
{
public:
	reader(const std::wstring &path);

	reader(const reader &) = delete;
	reader &operator=(const reader &) = delete;

	// A version cookie for the entire file:
	uint32_t version() { return _version; }
	void set_version(uint32_t version) { _version = version; }

	template<class T>
	std::enable_if_t<std::is_pod<T>::value>
		read_raw(T &data)
	{
		_stream.read((char *)&data, sizeof(T));
	}

	void read_raw(uint8_t *data, std::size_t data_size)
	{
		_stream.read((char *)data, data_size);
	}

	void skip(std::size_t bytes)
	{
		_stream.seekg(bytes, std::ios_base::cur);
	}

	void seek(std::size_t bytes)
	{
		_stream.seekg(bytes, std::ios_base::beg);
	}

	std::streampos position()
	{
		return _stream.tellg();
	}

	std::size_t scan_for_terminator(uint8_t terminator);
	std::size_t scan_for_terminator(uint8_t terminator, uint32_t maximum_size);

	bool at_end()
	{
		return _stream.eof();
	}

	std::streampos remaining()
	{
		return _stream_size - _stream.tellg();
	}

private:
	std::ifstream _stream;
	std::streampos _stream_size;
	uint32_t _version { 0 };
};

}
}
