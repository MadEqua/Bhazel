project "ImGui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"

    targetdir "../../bin/%{OUTPUT_DIR}/%{prj.name}"
    objdir "../../bin-int/%{OUTPUT_DIR}/%{prj.name}"

    files {
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
