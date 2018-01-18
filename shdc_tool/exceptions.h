#pragma once

class application_exception
{
public:
	application_exception(const std::string &message)
		: _message(message)
	{
	}

	application_exception(std::string &&message)
		: _message(std::move(message))
	{
	}

	const std::string &message() const { return _message; }

private:
	std::string _message;
};

class format_exception
{
public:
	format_exception(const std::string &message)
		: _message(message)
	{
	}

	format_exception(std::string &&message)
		: _message(std::move(message))
	{
	}

	const std::string &message() const { return _message; }

private:
	std::string _message;
};

class truncated_variable_size_field : public format_exception
{
public:
	truncated_variable_size_field()
		: format_exception("A size field in the file specified a length that exceeds the end of the file.")
	{
	}
};
