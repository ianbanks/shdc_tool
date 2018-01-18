#pragma once

namespace arma {
namespace io {

static inline reader &operator>>(reader &reader, uint32_t &value)
{
	reader.read_raw(value);

	return reader;
}

static inline reader &operator>>(reader &reader, uint16_t &value)
{
	reader.read_raw(value);

	return reader;
}

static inline reader &operator>>(reader &reader, uint8_t &value)
{
	reader.read_raw(value);

	return reader;
}

static inline reader &operator>>(reader &reader, int32_t &value)
{
	reader.read_raw(value);

	return reader;
}

static inline reader &operator>>(reader &reader, int16_t &value)
{
	reader.read_raw(value);

	return reader;
}

static inline reader &operator>>(reader &reader, int8_t &value)
{
	reader.read_raw(value);

	return reader;
}

static inline reader &operator>>(reader &reader, float &value)
{
	reader.read_raw(value);

	return reader;
}

static inline reader &operator>>(reader &reader, bool &value)
{
	uint8_t packed_value;

	reader.read_raw(packed_value);

	value = packed_value != 0;

	return reader;
}

struct prefixed_string
{
	prefixed_string(std::string &referenced)
	: referenced_string(referenced)
	{
	}

	std::string &referenced_string;
};

struct zero_terminated_string
{
	zero_terminated_string(std::string &referenced)
	: referenced_string(referenced)
	{
	}

	std::string &referenced_string;
};

static inline reader &operator>>(reader &reader, zero_terminated_string &manipulator)
{
	std::size_t string_size = reader.scan_for_terminator(0);

	manipulator.referenced_string.resize(string_size);

	reader.read_raw((uint8_t *)manipulator.referenced_string.data(), string_size);
	reader.skip(sizeof(uint8_t));

	return reader;
}

struct sized_string
{
	sized_string(std::string &referenced, uint32_t size)
	: referenced_string(referenced), size(size)
	{
	}

	std::string &referenced_string;
	uint32_t size;
};

static inline reader &operator>>(reader &reader, sized_string &manipulator)
{
	std::size_t string_size = reader.scan_for_terminator(0, manipulator.size);

	manipulator.referenced_string.resize(string_size);

	reader.read_raw((uint8_t *)manipulator.referenced_string.data(), string_size);
	reader.skip(manipulator.size - string_size);

	return reader;
}

template <typename T>
struct sized_vector
{
	sized_vector(std::vector<T> &referenced, uint32_t size)
	: referenced_vector(referenced), size(size)
	{
	}

	std::vector<T> &referenced_vector;
	uint32_t size;
};

template <typename T>
static inline reader &operator>>(reader &reader, sized_vector<T> &manipulator)
{
	if (manipulator.size == 0) return reader;

	std::streampos data_length = sizeof(T) * manipulator.size;

	if (reader.remaining() < data_length) throw truncated_variable_size_field();

	manipulator.referenced_vector.resize(manipulator.size);

	reader.read_raw((uint8_t *)manipulator.referenced_vector.data(), (std::size_t)data_length);

	return reader;
}

template <typename T>
struct sized_class_vector
{
	sized_class_vector(std::vector<T> &referenced, uint32_t size)
	: referenced_vector(referenced), size(size)
	{
	}

	std::vector<T> &referenced_vector;
	uint32_t size;
};

template <typename T>
static inline reader &operator>>(reader &reader, sized_class_vector<T> &manipulator)
{
	if (manipulator.size == 0) return reader;

	// Avoid large reservations if it is unlikely the reader won't fail for truncation:
	if (manipulator.size * sizeof(T) <= reader.remaining()) manipulator.referenced_vector.reserve(manipulator.size);

	for (uint32_t i = 0; i < manipulator.size; i++)
	{
		manipulator.referenced_vector.emplace_back(reader);
	}

	return reader;
}

template <typename T>
struct sized_array
{
	sized_array(T *referenced, uint32_t size)
	: referenced_array(referenced), size(size)
	{
	}

	T *referenced_array;
	uint32_t size;
};

template <typename T>
static inline reader &operator>>(reader &reader, sized_array<T> &manipulator)
{
	if (manipulator.size == 0) return reader;

	std::streampos data_length = sizeof(T) * manipulator.size;

	if (reader.remaining() < data_length) throw truncated_variable_size_field();

	reader.read_raw((uint8_t *)manipulator.referenced_array, (std::size_t)data_length);

	return reader;
}

template <typename T>
struct prefixed_vector
{
	prefixed_vector(std::vector<T> &referenced)
	: referenced_vector(referenced)
	{
	}

	std::vector<T> &referenced_vector;
};

template <typename T>
static inline reader &operator>>(reader &reader, prefixed_vector<T> &manipulator)
{
	uint32_t count;

	reader.read_raw(count);

	if (count == 0) return reader;

	std::streampos data_length = sizeof(T) * count;

	if (reader.remaining() < data_length) throw truncated_variable_size_field();

	manipulator.referenced_vector.resize(count);

	reader.read_raw((uint8_t *)manipulator.referenced_vector.data(), (std::size_t)data_length);

	return reader;
}

template <typename T>
struct prefixed_class_vector
{
	prefixed_class_vector(std::vector<T> &referenced)
	: referenced_vector(referenced)
	{
	}

	std::vector<T> &referenced_vector;
};

template <typename T>
static inline reader &operator>>(reader &reader, prefixed_class_vector<T> &manipulator)
{
	uint32_t count;

	reader.read_raw(count);

	manipulator.referenced_vector.clear();

	// Avoid large reservations if it is unlikely the reader won't fail for truncation:
	if (count * sizeof(T) <= reader.remaining()) manipulator.referenced_vector.reserve(count);

	for (uint32_t i = 0; i < count; i++)
	{
		manipulator.referenced_vector.emplace_back(reader);
	}

	return reader;
}

}
}