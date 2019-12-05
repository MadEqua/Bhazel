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