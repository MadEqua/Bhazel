workspace "Bhazel"
    architecture "x64"
    startproject "Sandbox"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

outputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Bhazel/vendor/glad"
include "Bhazel/vendor/glfw_premake5.lua"
include "Bhazel/vendor/imgui_premake5.lua"
include "Bhazel/vendor/stb_image"


project "Bhazel"
    location "Bhazel"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir("bin/" .. outputDir .. "/%{prj.name}")
    objdir("bin-int/" .. outputDir .. "/%{prj.name}")

    pchheader "bzpch.h"
    pchsource "Bhazel/src/bzpch.cpp"

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
    }

    includedirs
    {
        "%{prj.name}/src",
        "%{prj.name}/vendor/spdlog/include",
        "%{prj.name}/vendor/GLFW/include",
        "%{prj.name}/vendor/glad/include",
        "%{prj.name}/vendor/ImGui",
        "%{prj.name}/vendor/glm",
        "%{prj.name}/vendor/stb_image"
    }

    links
    {
        "GLFW",
        "glad",
        "ImGui",
        "stb_image",
        "opengl32.lib"
    }

    defines
    {
        "GLFW_INCLUDE_NONE",
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "BZ_PLATFORM_WINDOWS",
        }

        links
        {
            "d3d11.lib",
            "dxguid.lib",
            "D3DCompiler.lib",
        }

    filter "configurations:Debug"
        defines "BZ_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "BZ_RELEASE"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        defines "BZ_DIST"
        runtime "Release"
        optimize "on"



project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    staticruntime "on"
    language "C++"
    cppdialect "C++17"

    targetdir("bin/" .. outputDir .. "/%{prj.name}")
    objdir("bin-int/" .. outputDir .. "/%{prj.name}")
    debugdir("bin/" .. outputDir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "%{prj.name}/src",
        "Bhazel/src",
        "Bhazel/vendor/spdlog/include",
        "Bhazel/vendor/glm",
        "Bhazel/vendor/ImGui",
    }

    links
    {
        "Bhazel"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "BZ_PLATFORM_WINDOWS",
        }

    filter "configurations:Debug"
        defines "BZ_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "BZ_RELEASE"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        defines "BZ_DIST"
        runtime "Release"
        optimize "on"