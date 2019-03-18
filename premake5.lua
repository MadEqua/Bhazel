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

include "Bhazel/vendor/GLFW"
include "Bhazel/vendor/glad"
include "Bhazel/vendor/imgui"

project "Bhazel"
    location "Bhazel"
    kind "SharedLib"
    language "C++"
    staticruntime "off"

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
        cppdialect "C++17"
        systemversion "latest"

        defines
        {
            "BZ_PLATFORM_WINDOWS",
            "BZ_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
        }

        postbuildcommands
        {
            ("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputDir .. "/Sandbox/\"")
        }

    filter "configurations:Debug"
        defines "BZ_DEBUG"
		runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines "BZ_RELEASE"
		runtime "Release"
        optimize "On"

    filter "configurations:Dist"
        defines "BZ_DIST"
		runtime "Release"
        optimize "On"



project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    staticruntime "off"

    language "C++"

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
        "%{includeDir.glm}"
    }

    links
    {
        "Bhazel"
    }

    filter "system:windows"
        cppdialect "C++17"
        systemversion "latest"

        defines
        {
            "BZ_PLATFORM_WINDOWS",
        }

    filter "configurations:Debug"
        defines "BZ_DEBUG"
		runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines "BZ_RELEASE"
		runtime "Release"
        optimize "On"

    filter "configurations:Dist"
        defines "BZ_DIST"
		runtime "Release"
        optimize "On"