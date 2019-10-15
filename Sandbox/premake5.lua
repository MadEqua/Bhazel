project "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"

    targetdir("../bin/" .. outputDir .. "/%{prj.name}")
    objdir("../bin-int/" .. outputDir .. "/%{prj.name}")
    debugdir("../bin/" .. outputDir .. "/%{prj.name}")

    files
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
        "src",
        "../Bhazel/src",
        "../Bhazel/vendor/spdlog/include",
        "../Bhazel/vendor/glm",
        "../Bhazel/vendor/ImGui",
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