#pragma once

namespace filesystem
{

static inline bool is_file(const std::wstring &path)
{
	struct _stat64i32 information;

	if (_wstat(path.c_str(), &information) != 0) return false;

	return (information.st_mode & S_IFDIR) == 0;
}

static inline bool is_directory(const std::wstring &path)
{
	struct _stat64i32 information;

	if (_wstat(path.c_str(), &information) != 0) return false;

	return (information.st_mode & S_IFDIR) != 0;
}

}
