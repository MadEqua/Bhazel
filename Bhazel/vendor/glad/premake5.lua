project "glad"
    kind "StaticLib"
    language "C"

    targetdir ("../bin/" .. outputDir .. "/%{prj.name}")
    objdir ("../bin-int/" .. outputDir .. "/%{prj.name}")

    files
    {
        "include/glad/glad.h",
        "include/KHR/khrplatform.h",
        "src/glad.c"
    }

    includedirs
    {
        "include"
    }

    removeplatforms
    { 
        "D3D11",
        "Vulkan"
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