workspace "Bhazel"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

outputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"



project "Bhazel"
    location "Bhazel"
    kind "SharedLib"
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
        "%{prj.name}/vendor/spdlog/include"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        defines
        {
            "BZ_PLATFORM_WINDOWS",
            "BZ_BUILD_DLL"
        }

        postbuildcommands
        {
            ("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputDir .. "/Sandbox")
        }

    filter "configurations:Debug"
        defines "BZ_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "BZ_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "BZ_DIST"
        optimize "On"



project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"

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
        "Bhazel/src"
    }

    links
    {
        "Bhazel"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        defines
        {
            "BZ_PLATFORM_WINDOWS",
        }

    filter "configurations:Debug"
        defines "BZ_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "BZ_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "BZ_DIST"
        optimize "On"