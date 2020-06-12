#include "GuiRoot.h"

#include "Panels/ViewportPanel.h"

#include <imgui.h>


namespace BZ {

GuiRoot::GuiRoot() {
    panels.emplace_back(std::make_unique<ViewportPanel>());
}

void GuiRoot::render() {
    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("RootWindow", 0, windowFlags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("DockSpace");
    ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspaceFlags);

    renderMenuBar();

    for (auto &panel : panels) {
        panel->render();
    }

    ImGui::End();
}

void GuiRoot::renderMenuBar() {
    if (ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open...", "Ctrl+O")) {
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
            }
            if (ImGui::MenuItem("Close", "Ctrl+W")) {
            }
            ImGui::EndMenu();
        }

        // if (greenOrange.hasOpenProject() && ImGui::BeginMenu("Panels")) {
        //    for (auto &panelPtr : panels) {
        //        if (ImGui::MenuItem(panelPtr->getName(), 0, panelPtr->isOpen())) {
        //            panelPtr->flipOpen();
        //        }
        //    }
        //    ImGui::EndMenu();
        //}

        bool openAbout = false;
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                openAbout = true;
            }
            ImGui::EndMenu();
        }

        if (openAbout)
            ImGui::OpenPopup("About");

        // ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
        if (ImGui::BeginPopup("About")) {
            ImGui::Text("BhazelEd <insert version>");
            ImGui::Separator();
            if (ImGui::Button("Awesome!", ImVec2(120, 0)))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

#ifndef BZ_DIST
        static bool showDemoWindow = false;
        if (ImGui::BeginMenu("ImGui Demo")) {
            showDemoWindow = !showDemoWindow;
            ImGui::EndMenu();
        }
        if (showDemoWindow)
            ImGui::ShowDemoWindow(&showDemoWindow);
#endif
        ImGui::EndMainMenuBar();
    }
}

}