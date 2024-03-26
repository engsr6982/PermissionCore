#pragma once

#ifdef PERMISSION_CORE_API_EXPORT

#define PermExports __declspec(dllexport)

#else

#define PermExports __declspec(dllimport)

#endif