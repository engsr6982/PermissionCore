#pragma once

#ifdef PERMISSION_CORE_API_EXPORT

#define PermExports __declspec(dllexport)

#else

#define PermExports __declspec(dllimport)

#endif

/**

您不应该使用此头文件
也不该定义、使用此文件中的仍和一个宏

*/