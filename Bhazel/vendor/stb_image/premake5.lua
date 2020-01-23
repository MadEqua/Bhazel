project "stb_image"
    kind "StaticLib"
    language "C"

    targetdir "../bin/%{OUTPUT_DIR}/%{prj.name}"
    objdir "../bin-int/%{OUTPUT_DIR}/%{prj.name}"

    files {
        "stb_image.h",
        "stb_image.cpp",
    }

    includedirs {
    }