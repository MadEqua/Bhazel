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
        "vendor/glad/include",
        "vendor/ImGui",
        "vendor/glm",
        "vendor/stb_image",
        "vendor/VulkanMemoryAllocator",
        "vendor/tinyobjloader",
        "%{VULKAN_SDK_DIR}/Include"
    }

    libdirs {
        "%{VULKAN_SDK_DIR}/Lib"
    }

    links {
        "GLFW",
        "glad",
        "ImGui",
        "stb_image",
        "vulkan-1.lib",
        "opengl32.lib",
    }

    defines {
        "GLFW_INCLUDE_NONE",
    }

    filter "system:windows"
        links {
            "d3d11.lib",
            "dxguid.lib",
            "D3DCompiler.lib",
        }