project "stb_image"
    kind "StaticLib"
    language "C"

    targetdir ("../bin/" .. outputDir .. "/%{prj.name}")
    objdir ("../bin-int/" .. outputDir .. "/%{prj.name}")

    files
    {
        "stb_image.h",
        "stb_image.cpp",
    }

    includedirs
    {
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
        optimize "off"

    filter "configurations:Release"
        runtime "Release"
        symbols "off"
        optimize "on"