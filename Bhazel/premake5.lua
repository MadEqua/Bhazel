project "Bhazel"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"

    targetdir "../bin/%{OUTPUT_DIR}/%{prj.name}"
    objdir "../bin-int/%{OUTPUT_DIR}/%{prj.name}"

    pchheader "bzpch.h"
    pchsource "src/bzpch.cpp"

    files {
        "src/**.h",
        "src/**.cpp",
    }

    includedirs {
        "src",
        "vendor/spdlog/include",
        "vendor/GLFW/include",
        "vendor/ImGui",
        "vendor/glm",
        "vendor/stb_image",
        "vendor/VulkanMemoryAllocator",
        "vendor/tinyobjloader",
        "vendor/filewatch",
        "vendor/entt/src",
        "%{VULKAN_SDK_DIR}/Include"
    }

    libdirs {
        "%{VULKAN_SDK_DIR}/Lib"
    }

    links {
        "GLFW",
        "ImGui",
        "stb_image",
        "vulkan-1.lib",
    }

    defines {
        "GLFW_INCLUDE_NONE",
    }