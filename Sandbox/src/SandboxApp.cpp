#include "SandboxApp.h"

#include <imgui.h>


ParticleLayer::ParticleLayer() : Layer("Particle") {
}

void ParticleLayer::onAttachToEngine() {
    const auto &WINDOW_DIMS = BZ::Engine::get().getWindow().getDimensionsFloat();
    const glm::vec2 WINDOW_HALF_DIMS = WINDOW_DIMS * 0.5f;
    camera = BZ::OrthographicCamera(-WINDOW_HALF_DIMS.x, WINDOW_HALF_DIMS.x, -WINDOW_HALF_DIMS.y, WINDOW_HALF_DIMS.y);
    camera.getTransform().setTranslation(WINDOW_HALF_DIMS.x, WINDOW_HALF_DIMS.y, 0.0f, BZ::Space::Parent);
    cameraController = BZ::CameraController2D(camera, 400.0f, true, 45.0f);

    tex1 = BZ::Texture2D::create("Sandbox/textures/alphatest.png", VK_FORMAT_R8G8B8A8_SRGB,
                                 BZ::MipmapData::Options::Generate);
    tex2 = BZ::Texture2D::create("Sandbox/textures/particle.png", VK_FORMAT_R8G8B8A8_SRGB,
                                 BZ::MipmapData::Options::Generate);

    particleSystem.setPosition({ WINDOW_HALF_DIMS.x, WINDOW_HALF_DIMS.y });
    BZ::Particle2DRanges ranges;
    ranges.lifeSecsRange = { 3.0f, 6.0f };
    ranges.dimensionRange = { { 5.0f, 5.0f }, { 10.0f, 10.0f } };
    ranges.tintAndAlphaRange = { { 0.1f, 0.1f, 0.7f, 1.0f }, { 0.2f, 0.5f, 0.9f, 1.0f } };
    ranges.velocityRange = { { -200.0f, 100.0f }, { 200.0f, 500.0f } };
    ranges.angularVelocityRange = { -100.0f, 100.0f };
    ranges.accelerationRange = { 0.0f, -100.0f };

    BZ::Particle2DRanges ranges2;
    ranges2.lifeSecsRange = { 1.0f, 3.0f };
    ranges2.dimensionRange = { { 5.0f, 5.0f }, { 11.0f, 11.0f } };
    ranges2.tintAndAlphaRange = { 1.0f, 1.0f, 1.0f, 1.0f };
    ranges2.velocityRange = { { -60.0f, -200.0f }, { 60.0f, -100.0f } };
    ranges2.angularVelocityRange = { -300.0f, 300.0f };
    ranges2.accelerationRange = { 0.0f, -100.0f };

    particleSystem.addEmitter({ 0.0f, 0.0f }, 3'000, -1, ranges2, tex1);
    particleSystem.addEmitter({ 0.0f, 0.0f }, 5'000, -1, ranges, tex2);

    particleSystem.start();
}

void ParticleLayer::onUpdate(const BZ::FrameTiming &frameTiming) {
    BZ_PROFILE_FUNCTION();

    auto WINDOW_DIMS = BZ::Engine::get().getWindow().getDimensions();

    cameraController.onUpdate(frameTiming);

    static glm::vec2 pos = { 0.0f, WINDOW_DIMS.y * 0.5f };
    static bool right = true;

    if (right && pos.x >= WINDOW_DIMS.x)
        right = false;
    else if (!right && pos.x <= 0.0f)
        right = true;
    float mult = right ? 1.0f : -1.0f;

    pos.x += 100.0f * frameTiming.deltaTime.asSeconds() * mult;
    pos.y = (sin(pos.x * 0.02f) * 0.5f + 0.5f) * (WINDOW_DIMS.y * 0.75f) + (WINDOW_DIMS.y * 0.125f);
    particleSystem.setPosition(pos);

    BZ::Renderer2D::begin(camera);
    particleSystem.onUpdate(frameTiming);
    BZ::Renderer2D::renderParticleSystem2D(particleSystem);
    BZ::Renderer2D::end();
}

void ParticleLayer::onEvent(BZ::Event &event) {
    cameraController.onEvent(event);
}

void ParticleLayer::onImGuiRender(const BZ::FrameTiming &frameTiming) {
    BZ_PROFILE_FUNCTION();
}


Layer3D::Layer3D() : Layer("3D") {
}


void Layer3D::onAttachToEngine() {
    const auto &WINDOW_DIMS = BZ::Engine::get().getWindow().getDimensionsFloat();
    const glm::vec2 WINDOW_HALF_DIMS = WINDOW_DIMS * 0.5f;
    orthoCamera =
        BZ::OrthographicCamera(-WINDOW_HALF_DIMS.x, WINDOW_HALF_DIMS.x, -WINDOW_HALF_DIMS.y, WINDOW_HALF_DIMS.y);

    camera = BZ::PerspectiveCamera(50.0f, BZ::Engine::get().getWindow().getAspectRatio(), 0.1f, 600.0f);
    camera.getTransform().setTranslation(0.0f, 100.0f, 100.0f, BZ::Space::Parent);
    camera.setExposure(0.5f);
    scenes[0] = new BZ::Scene(camera);
    scenes[1] = new BZ::Scene(camera);
    scenes[2] = new BZ::Scene(camera);
    rotateCameraController = BZ::RotateCameraController(camera, 11.0f, 0.07f);
    freeCameraController = BZ::FreeCameraController(camera, 50.0f);

    // Hydrant, Wrench and Cerberus
#if 1
    BZ::Material hydrantMaterial("Sandbox/meshes/fireHydrant/BaseColor.png", "Sandbox/meshes/fireHydrant/Normal.png",
                                 "Sandbox/meshes/fireHydrant/Metallic.png", "Sandbox/meshes/fireHydrant/Roughness.png",
                                 "Sandbox/meshes/fireHydrant/Height.png");
    hydrantMaterial.setParallaxOcclusionScale(0.001f);
    BZ::Mesh hydrantMesh("Sandbox/meshes/fireHydrant/fireHydrant.obj", hydrantMaterial);
    BZ::Transform hydrantTransform;
    hydrantTransform.setScale(0.5f, 0.5f, 0.5f);
    hydrantTransform.setTranslation(0.0f, -26.0f, 0.0f, BZ::Space::Parent);
    scenes[0]->addEntity(hydrantMesh, hydrantTransform);
    scenes[1]->addEntity(hydrantMesh, hydrantTransform);
    scenes[2]->addEntity(hydrantMesh, hydrantTransform);

    BZ::Material wrenchMaterial("Sandbox/meshes/wrench/albedo.jpg", "Sandbox/meshes/wrench/normal.png",
                                "Sandbox/meshes/wrench/metallic.jpg", "Sandbox/meshes/wrench/roughness.jpg",
                                "Sandbox/meshes/wrench/height.png");
    wrenchMaterial.setParallaxOcclusionScale(0.01f);
    BZ::Mesh wrenchMesh("Sandbox/meshes/wrench/wrench.obj", wrenchMaterial);
    BZ::Transform wrenchTransform;
    wrenchTransform.setTranslation(20.0f, 0.0f, 0.0f, BZ::Space::Parent);
    wrenchTransform.setRotationEuler(0.0f, 90.0f, 30.0f, BZ::Space::Parent);
    scenes[0]->addEntity(wrenchMesh, wrenchTransform);
    scenes[1]->addEntity(wrenchMesh, wrenchTransform);
    scenes[2]->addEntity(wrenchMesh, wrenchTransform);

    BZ::Material gunMaterial("Sandbox/meshes/cerberus/albedo.png", "Sandbox/meshes/cerberus/normal.png",
                             "Sandbox/meshes/cerberus/metallic.png", "Sandbox/meshes/cerberus/roughness.png",
                             "Sandbox/meshes/cerberus/height.png");
    gunMaterial.setParallaxOcclusionScale(0.0f);
    BZ::Mesh gunMesh("Sandbox/meshes/cerberus/cerberus.obj", gunMaterial);
    BZ::Transform gunTransform;
    gunTransform.setScale(9.0f, 9.0f, 9.0f);
    gunTransform.setTranslation(-25.0f, 0.0f, 0.0f, BZ::Space::Parent);
    // gunTransform.setRotationEuler(0.0f, 90.0f, -45.0f, BZ::Space::Parent);
    scenes[0]->addEntity(gunMesh, gunTransform);
    scenes[1]->addEntity(gunMesh, gunTransform);
    scenes[2]->addEntity(gunMesh, gunTransform);
#endif

    // Ground
#if 1
    BZ::Material groundMaterial("Sandbox/textures/steppingstones/steppingstones1_albedo.png",
                                "Sandbox/textures/steppingstones/steppingstones1_normal.png",
                                "Sandbox/textures/steppingstones/steppingstones1_metallic.png",
                                "Sandbox/textures/steppingstones/steppingstones1_roughness.png",
                                "Sandbox/textures/steppingstones/steppingstones1_height.png", nullptr, true);

    // BZ::Material groundMaterial("Sandbox/textures/octostone/octostoneAlbedo.png",
    //                            "Sandbox/textures/octostone/octostoneNormalc.png",
    //                            "Sandbox/textures/octostone/octostoneMetallic.png",
    //                            "Sandbox/textures/octostone/octostoneRoughness2.png",
    //                            "Sandbox/textures/octostone/octostoneHeightc.png",
    //                            "Sandbox/textures/octostone/octostoneAmbient_Occlusionc.png");
    // BZ::Material groundMaterial("Sandbox/textures/hexstones/hex-stones1-albedo.png",
    //                            "Sandbox/textures/hexstones/hex-stones1-normal-ogl.png",
    //                            "Sandbox/textures/hexstones/hex-stones1-metallic.png",
    //                            "Sandbox/textures/hexstones/hex-stones1-roughness.png",
    //                            "Sandbox/textures/hexstones/hex-stones1-height.png");
    groundMaterial.setParallaxOcclusionScale(0.15f);
    groundMaterial.setUvScale(20.0f, 20.0f);
    BZ::Mesh groundMesh = BZ::Mesh::createHorizontalPlane(groundMaterial);
    BZ::Transform groundTransform;
    groundTransform.setTranslation(0.0f, -25.0f, 0.0f, BZ::Space::Parent);
    groundTransform.setScale(300.0f, 300.0f, 300.0f);
    scenes[0]->addEntity(groundMesh, groundTransform, false);
    scenes[1]->addEntity(groundMesh, groundTransform, false);
    scenes[2]->addEntity(groundMesh, groundTransform, false);
#endif

    // Houses and barrel
#if 0
    BZ::Mesh barrelMesh("Sandbox/meshes/barrel/barrel.obj");
    BZ::Transform barrelTransform;
    barrelTransform.setScale(2.5f, 2.5f, 2.5f);
    barrelTransform.setTranslation(40.0f, -23.0f, 0.0f, BZ::Space::Parent);
    barrelTransform.setRotationEuler(29.0f, 4.0f, 90.0f, BZ::Space::Parent);
    scenes[0]->addEntity(barrelMesh, barrelTransform);
    scenes[1]->addEntity(barrelMesh, barrelTransform);
    scenes[2]->addEntity(barrelMesh, barrelTransform);

    BZ::Mesh houseMesh("Sandbox/meshes/house/house.obj");
    BZ::Transform houseTransform;
    houseTransform.setScale(10.0f, 10.0f, 10.0f);
    houseTransform.setTranslation(0.4f, -29.0f, 0.0f, BZ::Space::Parent);
    houseTransform.setRotationEuler(0.0f, 295.0f, 0.0f, BZ::Space::Parent);
    scenes[0]->addEntity(houseMesh, houseTransform);
    scenes[1]->addEntity(houseMesh, houseTransform);
    scenes[2]->addEntity(houseMesh, houseTransform);
    
    BZ::Transform houseTransform2;
    houseTransform2.setScale(10.0f, 10.0f, 10.0f);
    houseTransform2.setTranslation(-83.0f, -29.0f, -27.4f, BZ::Space::Parent);
    houseTransform2.setRotationEuler(0.0f, -218.0f, 0.0f, BZ::Space::Parent);
    scenes[0]->addEntity(houseMesh, houseTransform2);
    scenes[1]->addEntity(houseMesh, houseTransform2);
    scenes[2]->addEntity(houseMesh, houseTransform2);
    
    BZ::Transform houseTransform3;
    houseTransform3.setScale(10.0f, 10.0f, 10.0f);
    houseTransform3.setTranslation(-20.0f, -29.0f, -100.0f, BZ::Space::Parent);
    houseTransform3.setRotationEuler(0.0f, 151.0f, 0.0f, BZ::Space::Parent);
    scenes[0]->addEntity(houseMesh, houseTransform3);
    scenes[1]->addEntity(houseMesh, houseTransform3);
    scenes[2]->addEntity(houseMesh, houseTransform3);
#endif

    // Sphere Wall
#if 0
    byte whiteTextureData [] = { 255, 255, 255, 255 };
    auto texRef = BZ::Texture2D::create(whiteTextureData, 1, 1, VK_FORMAT_R8G8B8A8_UNORM, BZ::MipmapData::Options::DoNothing);
    //auto texRef = BZ::Texture2D::create("Sandbox/textures/test.jpg", VK_FORMAT_R8G8B8A8_SRGB, BZ::MipmapData::Options::Generate);
    BZ::Mesh sphereMesh("Sandbox/meshes/sphere.obj");

    const float SCALE = 7.0f;
    for (int x = -3; x <= 3; ++x) {
        for (int y = -3; y <= 3; ++y) {
            glm::vec3 pos(x * (SCALE * 2.4f), y * (SCALE * 2.2f), 0.0f);
    
            BZ::Transform sphereTransform;
            sphereTransform.setScale(SCALE);
            sphereTransform.setTranslation(pos, BZ::Space::Parent);
    
            BZ::Material mat(texRef);
            mat.setMetallic(static_cast<float>(x + 3) / 6.0f);
            mat.setRoughness(static_cast<float>(y + 3) / 6.0f);
    
            scenes[0]->addEntity(sphereMesh, sphereTransform, mat, false);
            scenes[1]->addEntity(sphereMesh, sphereTransform, mat, false);
            scenes[2]->addEntity(sphereMesh, sphereTransform, mat, false);
        }
    }
#endif

    BZ::DirectionalLight dirLight;
    dirLight.setDirection({ 0.3f, -1.0f, -1.0f });
    dirLight.color = { 1.0f, 1.0f, 1.0f };
    dirLight.intensity = 1.0f;
    scenes[0]->addDirectionalLight(dirLight);
    scenes[1]->addDirectionalLight(dirLight);
    scenes[2]->addDirectionalLight(dirLight);

    // BZ::DirectionalLight dirLight2;
    // dirLight2.setDirection({ 0.0f, -1.0f, -0.5f });
    // dirLight2.color = { 1.0f, 1.0f, 1.0f };
    // dirLight2.intensity = 1.0f;
    // scenes[0]->addDirectionalLight(dirLight2);
    // scenes[1]->addDirectionalLight(dirLight2);
    // scenes[2]->addDirectionalLight(dirLight2);

    const char *cubeFileNames[6] = { "skybox_posx.hdr", "skybox_negx.hdr", "skybox_posy.hdr",
                                     "skybox_negy.hdr", "skybox_posz.hdr", "skybox_negz.hdr" };

    scenes[0]->enableSkyBox("Sandbox/textures/umbrellas/", cubeFileNames, "Sandbox/textures/umbrellasIrradiance/",
                           cubeFileNames, "Sandbox/textures/umbrellasRadiance/", cubeFileNames, 5);
}

void Layer3D::onUpdate(const BZ::FrameTiming &frameTiming) {
    BZ_PROFILE_FUNCTION();

    if (useFreeCamera)
        freeCameraController.onUpdate(frameTiming);
    else
        rotateCameraController.onUpdate(frameTiming);

    // scenes[activeScene].getEntities()[2].transform.yaw(frameTiming.deltaTime.asSeconds() * 10.0f, BZ::Space::Local);

    BZ::Renderer::renderScene(*scenes[activeScene]);

    BZ::Renderer2D::begin(orthoCamera);
    // BZ::Renderer2D::renderQuad(glm::vec2(100.0f, 0.0f), glm::vec2(200.0f, 200.0f), 0.0f, glm::vec4(1, 1, 1, 1));

    const auto &WINDOW_DIMS = BZ::Engine::get().getWindow().getDimensionsFloat();
    const glm::vec2 SIZE = { 256, 256 };
    const glm::vec2 WINDOW_HALF_DIMS = WINDOW_DIMS * 0.5f;
    glm::vec2 pos = -WINDOW_HALF_DIMS + SIZE * 0.5f;

    BZ::Renderer2D::renderQuad(
        pos, SIZE, 0.0f,
        std::static_pointer_cast<BZ::Texture2D>(scenes[activeScene]
                                                    ->getDirectionalLights()[0]
                                                    .shadowMapFramebuffer->getDepthStencilTextureView()
                                                    ->getTexture()),
        glm::vec4(1, 1, 1, 1));

    BZ::Renderer2D::end();
}

void Layer3D::onEvent(BZ::Event &event) {
    if (useFreeCamera)
        freeCameraController.onEvent(event);
    else
        rotateCameraController.onEvent(event);
}

void Layer3D::onImGuiRender(const BZ::FrameTiming &frameTiming) {
    BZ_PROFILE_FUNCTION();

    if (ImGui::Begin("Scene Picker")) {
        const char *const items[] = { "Scene1", "Scene2", "Scene3" };
        if (ImGui::ListBox("", &activeScene, items, sizeof(items) / sizeof(char *))) {
        }
    }
    ImGui::End();

    BZ::Scene &scene = *scenes[activeScene];

    int i = 0;
    if (ImGui::Begin("Transforms")) {
        for (auto &entity : scene.getEntities()) {
            auto translation = entity.transform.getTranslation();
            auto rot = entity.transform.getRotationEuler();
            auto scale = entity.transform.getScale();
            ImGui::PushID(i);
            if (ImGui::DragFloat3("Translation", &translation[0], 0.1f, -100.0f, 100.0f)) {
                entity.transform.setTranslation(translation, BZ::Space::Parent);
            }
            if (ImGui::DragFloat3("Rotation", &rot[0], 1.0f, -359.0f, 359.0f)) {
                entity.transform.setRotationEuler(rot, BZ::Space::Parent);
            }
            if (ImGui::DragFloat3("Scale", &scale[0], 0.05f, 0.0f, 100.0f)) {
                entity.transform.setScale(scale);
            }
            ImGui::Separator();
            ImGui::PopID();
            i++;
        }
    }
    ImGui::End();

    if (ImGui::Begin("DirLights")) {
        i = 0;
        for (auto &dirLight : scene.getDirectionalLights()) {
            auto dir = dirLight.getDirection();
            auto col = dirLight.color;
            auto intensity = dirLight.intensity;

            ImGui::PushID(i);
            if (ImGui::DragFloat3("Direction", &dir[0], 0.05f, -1.0f, 1.0f)) {
                dirLight.setDirection(dir);
            }
            if (ImGui::DragFloat3("Color", &col[0], 0.05f, 0.0f, 1.0f)) {
                dirLight.color = col;
            }
            if (ImGui::DragFloat("Intensity", &intensity, 0.05f, 0.0f, 10.0f)) {
                dirLight.intensity = intensity;
            }
            ImGui::Separator();
            ImGui::PopID();
            i++;
        }
    }
    ImGui::End();

    if (ImGui::Begin("Camera")) {
        float exposure = camera.getExposure();
        if (ImGui::DragFloat("Exposure", &exposure, 0.01f, 0.0f, 20.0f)) {
            camera.setExposure(exposure);
        }
        ImGui::Checkbox("Use Free Camera", &useFreeCamera);
    }
    ImGui::End();

    if (ImGui::Begin("Materials")) {
        i = 0;
        for (auto &entity : scene.getEntities()) {
            for (auto &submesh : entity.mesh.getSubmeshes()) {
                BZ::Material &mat = entity.overrideMaterial.isValid() ? entity.overrideMaterial : submesh.material;
                float scale = mat.getParallaxOcclusionScale();
                ImGui::PushID(i);

                if (!mat.hasHeightTexture()) {
                    ImGui::Text("No Height texture, hiding Parallax Occ. Scale.");
                }
                else {
                    if (ImGui::DragFloat("Parallax Occ. Scale", &scale, 0.001f, 0.0f, 2.0f)) {
                        mat.setParallaxOcclusionScale(scale);
                    }
                }

                glm::vec2 uvScale = mat.getUvScale();
                if (ImGui::DragFloat2("UV Scale", &uvScale[0], 0.1f, 0.0f, 50.0f)) {
                    mat.setUvScale(uvScale);
                }

                if (mat.hasMetallicTexture()) {
                    ImGui::Text("Metallic texture in use, hiding Metallic slider.");
                }
                else {
                    float met = mat.getMetallic();
                    if (ImGui::DragFloat("Metallic", &met, 0.01f, 0.0f, 1.0f)) {
                        mat.setMetallic(met);
                    }
                }

                if (mat.hasMetallicTexture()) {
                    ImGui::Text("Roughness texture in use, hiding Roughness slider.");
                }
                else {
                    float rough = mat.getRoughness();
                    if (ImGui::DragFloat("Roughness", &rough, 0.01f, 0.0f, 1.0f)) {
                        mat.setRoughness(rough);
                    }
                }

                ImGui::Separator();
                ImGui::PopID();
                i++;
            }
        }
    }
    ImGui::End();
}


BZ::Application *createApplication() {
    return new Sandbox();
}