#pragma once

#ifdef PERMISSION_CORE_API_EXPORT

#define PERMISSION_CORE_API __declspec(dllexport)

#else

#define PERMISSION_CORE_API __declspec(dllimport)

#endif