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

    ------------------------------------------------
    -- Windows
    ------------------------------------------------
    filter "system:windows"
        systemversion "latest"
        toolset "msc"

    ------------------------------------------------
    -- Configurations
    ------------------------------------------------
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