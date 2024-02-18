#pragma once

// Include this file instead of <document.h> because rapidjson is configurable
// with macros.
// For instance, we need the std::string enabled version, and don't
// want to include the #define in every file.

#define RAPIDJSON_HAS_STDSTRING 1
#include <document.h>
