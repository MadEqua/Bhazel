#pragma once

#include <iostream>
#include <memory>
#include <algorithm>
#include <functional>
#include <vector>
#include <unordered_map>

#include <sstream>
#include <string>

#include <glm/glm.hpp>

#ifdef BZ_PLATFORM_WINDOWS
    #include <Windows.h>
    #include <Windowsx.h>
#endif

#include "Bhazel/Core/SingletonPattern.h"
#include "Bhazel/Core/Types.h"
#include "Bhazel/Core/Log.h"
#include "Bhazel/Core/Core.h"