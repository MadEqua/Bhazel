project "ImGui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    
    targetdir ("bin/" .. outputDir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputDir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/imconfig.h",
        "%{prj.name}/imgui.h",
        "%{prj.name}/imgui.cpp",
        "%{prj.name}/imgui_draw.cpp",
        "%{prj.name}/imgui_internal.h",
        "%{prj.name}/imgui_widgets.cpp",
        "%{prj.name}/imstb_rectpack.h",
        "%{prj.name}/imstb_textedit.h",
        "%{prj.name}/imstb_truetype.h",
        "%{prj.name}/imgui_demo.cpp"
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