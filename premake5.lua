workspace "Bhazel"
    architecture "x64"
    startproject "Sandbox"

    configurations {
        "Debug",
        "Release",
        "Dist"
    }

    filter "configurations:Debug"
        defines "BZ_DEBUG"
        runtime "Debug"
        symbols "on"
        optimize "off"

    filter "configurations:Release"
        defines "BZ_RELEASE"
        runtime "Release"
        symbols "on"
        optimize "on"

    filter "configurations:Dist"
        defines "BZ_DIST"
        runtime "Release"
        symbols "off"
        optimize "on"

    filter "system:windows"
        systemversion "latest"
        defines  "BZ_PLATFORM_WINDOWS"

OUTPUT_DIR = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

VULKAN_SDK_DIR = os.getenv("VULKAN_SDK")

include "Bhazel"
include "Sandbox"
include "BrickBreaker"
include "Bhazel/vendor/glad"
include "Bhazel/vendor/glfw_premake5.lua"
include "Bhazel/vendor/imgui_premake5.lua"
include "Bhazel/vendor/stb_image"