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

-- Include directories relative to root folder (solution directory)
includeDir = {}
includeDir["GLFW"] = "Bhazel/vendor/GLFW/include"
includeDir["glad"] = "Bhazel/vendor/glad/include"
includeDir["ImGui"] = "Bhazel/vendor/imgui"
includeDir["glm"] = "Bhazel/vendor/glm"

include "Bhazel/vendor/glad"
include "Bhazel/vendor/glfw_premake5.lua"
include "Bhazel/vendor/imgui_premake5.lua"

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
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "%{prj.name}/vendor/spdlog/include",
        "%{prj.name}/src",
        "%{includeDir.GLFW}",
        "%{includeDir.glad}",
        "%{includeDir.ImGui}",
        "%{includeDir.glm}"
    }

    links
    {
        "GLFW",
        "glad",
        "ImGui",
        "opengl32.lib"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "BZ_PLATFORM_WINDOWS",
            "BZ_BUILD_DLL",
            "GLFW_INCLUDE_NONE"
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

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "Bhazel/vendor/spdlog/include",
        "Bhazel/src",
        "Bhazel/vendor",
        "%{includeDir.glm}"
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