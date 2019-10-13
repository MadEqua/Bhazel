project "Bhazel"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"

    targetdir("../bin/" .. outputDir .. "/%{prj.name}")
    objdir("../bin-int/" .. outputDir .. "/%{prj.name}")

    pchheader "bzpch.h"
    pchsource "src/bzpch.cpp"

    files
    {
        "src/**.h",
        "src/**.cpp",
    }

    excludes
    { 
        "src/Platform/**.*"
    }

    includedirs
    {
        "src",
        "vendor/spdlog/include",
        "vendor/ImGui",
        "vendor/glm",
        "vendor/stb_image"
    }

    links
    {
        "ImGui",
        "stb_image"
    }

    ------------------------------------------------
    -- Vulkan
    ------------------------------------------------
    filter "platforms:Vulkan"
        files
        {
            "src/Platform/Vulkan/**.h",
            "src/Platform/Vulkan/**.cpp",

            "src/Platform/GLFW/**.h",
            "src/Platform/GLFW/**.cpp",
        }

        defines
        {
            "BZ_PLATFORM_VULKAN",
            "GLFW_INCLUDE_NONE"
        }

        includedirs
        {
            "vendor/GLFW/include",
            "%{VULKAN_SDK_DIR}/Include"
        }

        links
        {
            "GLFW",
            "vulkan-1.lib"
        }

        libdirs
        {
            "%{VULKAN_SDK_DIR}/Lib"
        }

    ------------------------------------------------
    -- OpenGL43
    ------------------------------------------------
    filter "platforms:OpenGL43"
        files
        {
            "src/Platform/OpenGL43/**.h",
            "src/Platform/OpenGL43/**.cpp",

            "src/Platform/GLFW/**.h",
            "src/Platform/GLFW/**.cpp",
        }

        defines
        {
            "BZ_PLATFORM_OPENGL43",
            "GLFW_INCLUDE_NONE"
        }

        includedirs
        {
            "vendor/GLFW/include",
            "vendor/glad/include"
        }

        links
        {
            "GLFW",
            "glad",
            "opengl32.lib"
        }

    ------------------------------------------------
    -- D3D11
    ------------------------------------------------
    filter "platforms:D3D11"
        files
        {
            "src/Platform/D3D11/**.h",
            "src/Platform/D3D11/**.cpp",

            "src/Platform/Win32/**.h",
            "src/Platform/Win32/**.cpp",
        }
        defines
        {
            "BZ_PLATFORM_D3D11"
        }

        links
        {
            "d3d11.lib",
            "dxguid.lib",
            "D3DCompiler.lib"
        }

    ------------------------------------------------
    -- Windows
    ------------------------------------------------
    filter "system:windows"
        systemversion "latest"
        toolset "msc"

        defines
        {
            "BZ_SYSTEM_WINDOWS"
        }

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