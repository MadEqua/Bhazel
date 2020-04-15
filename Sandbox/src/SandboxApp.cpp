#include "SandboxApp.h"

#include <imgui.h>


ParticleLayer::ParticleLayer() :
    Layer("Particle") {
}

void ParticleLayer::onAttach() {
}

void ParticleLayer::onGraphicsContextCreated() {
    const auto &WINDOW_DIMS = application.getWindow().getDimensionsFloat();
    const glm::vec2 WINDOW_HALF_DIMS = WINDOW_DIMS * 0.5f;
    camera = BZ::OrthographicCamera(-WINDOW_HALF_DIMS.x, WINDOW_HALF_DIMS.x, -WINDOW_HALF_DIMS.y, WINDOW_HALF_DIMS.y);
    camera.getTransform().setTranslation({ WINDOW_HALF_DIMS.x, WINDOW_HALF_DIMS.y, 0.0f });
    cameraController = BZ::OrthographicCameraController(camera);

    tex1 = BZ::Texture2D::create("Sandbox/textures/alphatest.png", BZ::TextureFormat::R8G8B8A8_SRGB, BZ::MipmapData::Options::Generate);
    tex2 = BZ::Texture2D::create("Sandbox/textures/particle.png", BZ::TextureFormat::R8G8B8A8_SRGB, BZ::MipmapData::Options::Generate);

    particleSystem.setPosition({ WINDOW_HALF_DIMS.x, WINDOW_HALF_DIMS.y});
    BZ::Particle2DRanges ranges;
    ranges.lifeSecsRange = { 3.0f, 6.0f };
    ranges.dimensionRange = { { 5.0f, 5.0f }, { 10.0f, 10.0f } };
    ranges.tintAndAlphaRange = { {0.1f, 0.1f, 0.7f, 1.0f}, { 0.2f, 0.5f, 0.9f, 1.0f } };
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

void ParticleLayer::onUpdate(const BZ::FrameStats &frameStats) {
    BZ_PROFILE_FUNCTION();

    auto WINDOW_DIMS = application.getWindow().getDimensions();

    cameraController.onUpdate(frameStats);

    static glm::vec2 pos = { 0.0f, WINDOW_DIMS.y * 0.5f };
    static bool right = true;

    if (right && pos.x >= WINDOW_DIMS.x) right = false;
    else if (!right && pos.x <= 0.0f) right = true;
    float mult = right ? 1.0f : -1.0f;

    pos.x += 100.0f * frameStats.lastFrameTime.asSeconds() * mult;
    pos.y = (sin(pos.x * 0.02f) * 0.5f + 0.5f) * (WINDOW_DIMS.y * 0.75f) + (WINDOW_DIMS.y * 0.125f);
    particleSystem.setPosition(pos);

    BZ::Renderer2D::begin(cameraController.getCamera());
    particleSystem.onUpdate(frameStats);
    BZ::Renderer2D::drawParticleSystem2D(particleSystem);
    BZ::Renderer2D::end();
}

void ParticleLayer::onEvent(BZ::Event &event) {
    cameraController.onEvent(event);
}

void ParticleLayer::onImGuiRender(const BZ::FrameStats &frameStats) {
    BZ_PROFILE_FUNCTION();
}



Layer3D::Layer3D() :
    Layer("3D") {
}

void Layer3D::onAttach() {
}

void Layer3D::onGraphicsContextCreated() {
    camera = BZ::PerspectiveCamera(50.0f, application.getWindow().getAspectRatio(), 0.1f, 300.0f);
    camera.getTransform().setTranslation({ 0.0f, 50.0f, 50.0f });
    scenes[0].setCamera(camera);
    scenes[1].setCamera(camera);
    scenes[2].setCamera(camera);
    
    rotateCameraController = BZ::RotateCameraController(camera, 1.0f);
    freeCameraController = BZ::FreeCameraController(camera);

    //BZ::Material hydrantMaterial("Sandbox/meshes/fireHydrant/BaseColor.png", 
    //                             "Sandbox/meshes/fireHydrant/Normal.png",
    //                             "Sandbox/meshes/fireHydrant/Metallic.png",
    //                             "Sandbox/meshes/fireHydrant/Roughness.png",
    //                             "Sandbox/meshes/fireHydrant/Height.png");
    //hydrantMaterial.setParallaxOcclusionScale(0.001f);
    //BZ::Mesh hydrantMesh("Sandbox/meshes/fireHydrant/fireHydrant.obj", hydrantMaterial);
    //BZ::Transform hydrantTransform;
    //hydrantTransform.setScale(0.5f, 0.5f, 0.5f);
    //hydrantTransform.setTranslation(0.0f, -25.0f, 0.0f);
    //scenes[0].addEntity(hydrantMesh, hydrantTransform);
    //scenes[1].addEntity(hydrantMesh, hydrantTransform);
    //scenes[2].addEntity(hydrantMesh, hydrantTransform);

    BZ::Material wrenchMaterial("Sandbox/meshes/wrench/albedo.jpg",
                                "Sandbox/meshes/wrench/normal.png",
                                "Sandbox/meshes/wrench/metallic.jpg",
                                "Sandbox/meshes/wrench/roughness.jpg",
                                "Sandbox/meshes/wrench/height.png");
    wrenchMaterial.setParallaxOcclusionScale(0.01f);
    BZ::Mesh wrenchMesh("Sandbox/meshes/wrench/wrench.obj", wrenchMaterial);
    BZ::Transform wrenchTransform;
    wrenchTransform.setTranslation(20.0f, 0.0f, 0.0f);
    wrenchTransform.setRotationEuler(0.0f, 90.0f, 30.0f);
    scenes[0].addEntity(wrenchMesh, wrenchTransform);
    scenes[1].addEntity(wrenchMesh, wrenchTransform);
    scenes[2].addEntity(wrenchMesh, wrenchTransform);
    
    //BZ::Material groundMaterial("Sandbox/textures/steppingstones/steppingstones1_albedo.png",
    //                            "Sandbox/textures/steppingstones/steppingstones1_normal.png",
    //                            "Sandbox/textures/steppingstones/steppingstones1_metallic.png",
    //                            "Sandbox/textures/steppingstones/steppingstones1_roughness.png",
    //                            "Sandbox/textures/steppingstones/steppingstones1_height.png");
    //groundMaterial.setParallaxOcclusionScale(0.15f);
    ////BZ::Mesh groundMesh = BZ::Mesh::createUnitCube(groundMaterial);
    ////BZ::Mesh groundMesh = BZ::Mesh::createHorizontalPlane(groundMaterial);
    //BZ::Mesh sphere("Sandbox/meshes/sphere.obj", groundMaterial);
    //BZ::Transform sphereTransform;
    //sphereTransform.setTranslation(-30.0f, 0.0f, 0.0f);
    //sphereTransform.setScale(10.0f, 10.0f, 10.0f);
    //scenes[0].addEntity(sphere, sphereTransform);
    //scenes[1].addEntity(sphere, sphereTransform);
    //scenes[2].addEntity(sphere, sphereTransform);

    BZ::Material gunMaterial("Sandbox/meshes/cerberus/albedo.png",
                             "Sandbox/meshes/cerberus/normal.png",
                             "Sandbox/meshes/cerberus/metallic.png",
                             "Sandbox/meshes/cerberus/roughness.png",
                             "Sandbox/meshes/cerberus/height.png");
    gunMaterial.setParallaxOcclusionScale(0.0f);
    BZ::Mesh gunMesh("Sandbox/meshes/cerberus/cerberus.obj", gunMaterial);
    BZ::Transform gunTransform;
    gunTransform.setScale(9.0f, 9.0f, 9.0f);
    gunTransform.setTranslation(-25.0f, 0.0f, 0.0f);
    gunTransform.setRotationEuler(0.0f, 90.0f, -45.0f);
    scenes[0].addEntity(gunMesh, gunTransform);
    scenes[1].addEntity(gunMesh, gunTransform);
    scenes[2].addEntity(gunMesh, gunTransform);
    
    BZ::DirectionalLight dirLight;
    dirLight.direction = { 1.0f, -1.0f, 0.0f };
    dirLight.color = { 1.0f, 1.0f, 1.0f };
    dirLight.intensity = 2.0f;
    scenes[0].addDirectionalLight(dirLight);
    scenes[1].addDirectionalLight(dirLight);
    scenes[2].addDirectionalLight(dirLight);

    const char* cubeFileNames[6] = { "output_skybox_posx.tga", "output_skybox_negx.tga", 
                                     "output_skybox_posy.tga", "output_skybox_negy.tga",
                                     "output_skybox_posz.tga", "output_skybox_negz.tga" };

    scenes[0].enableSkyBox("Sandbox/textures/lobby/", cubeFileNames,
                           "Sandbox/textures/lobbyIrradiance/", cubeFileNames,
                           "Sandbox/textures/lobbyRadiance/", cubeFileNames, 5);

    scenes[1].enableSkyBox("Sandbox/textures/theater/", cubeFileNames,
                           "Sandbox/textures/theaterIrradiance/", cubeFileNames,
                           "Sandbox/textures/theaterRadiance/", cubeFileNames, 5);

    scenes[2].enableSkyBox("Sandbox/textures/sky/", cubeFileNames,
                           "Sandbox/textures/skyIrradiance/", cubeFileNames,
                           "Sandbox/textures/skyRadiance/", cubeFileNames, 5);
}

void Layer3D::onUpdate(const BZ::FrameStats &frameStats) {
    BZ_PROFILE_FUNCTION();

    if(useFreeCamera)
        freeCameraController.onUpdate(frameStats);
    else
        rotateCameraController.onUpdate(frameStats);

    BZ::Renderer::drawScene(scenes[activeScene]);
}

void Layer3D::onEvent(BZ::Event &event) {
    if(useFreeCamera)
        freeCameraController.onEvent(event);
    else
        rotateCameraController.onEvent(event);
}

void Layer3D::onImGuiRender(const BZ::FrameStats &frameStats) {
    BZ_PROFILE_FUNCTION();

    if (ImGui::Begin("Scene Picker")) {
        const char * const items[] = { "Scene1", "Scene2", "Scene3" };
        if (ImGui::ListBox("", &activeScene, items, sizeof(items)/sizeof(char*))) {
        }
    }
    ImGui::End();

    BZ::Scene &scene = scenes[activeScene];

    if (ImGui::Begin("Transform")) {
        BZ::Entity &entity = scene.getEntities()[0];

        auto translation = entity.transform.getTranslation();
        auto rot = entity.transform.getRotationEuler();
        auto scale = entity.transform.getScale();

        if (ImGui::DragFloat3("Translation", &translation[0], 0.1f, -100.0f, 100.0f)) {
            entity.transform.setTranslation(translation);
        }
        if (ImGui::DragFloat3("Rot", &rot[0], 1.0f, -359.0f, 359.0f)) {
            entity.transform.setRotationEuler(rot);
        }
        if (ImGui::DragFloat3("Scale", &scale[0], 0.05f, 0.0f, 100.0f)) {
            entity.transform.setScale(scale);
        }
    }
    ImGui::End();

    if (ImGui::Begin("DirLight")) {
        auto dir = scene.getDirectionalLights()[0].direction;
        auto col = scene.getDirectionalLights()[0].color;
        auto intensity = scene.getDirectionalLights()[0].intensity;

        if (ImGui::DragFloat3("Direction", &dir[0], 0.05f, -1.0f, 1.0f)) {
            scene.getDirectionalLights()[0].direction = dir;
        }
        if (ImGui::DragFloat3("Color", &col[0], 0.05f, 0.0f, 1.0f)) {
            scene.getDirectionalLights()[0].color = col;
        }
        if (ImGui::DragFloat("Intensity", &intensity, 0.05f, 0.0f, 10.0f)) {
            scene.getDirectionalLights()[0].intensity = intensity;
        }
    }
    ImGui::End();

    if (ImGui::Begin("Camera")) {
        ImGui::Checkbox("Use Free Camera", &useFreeCamera);
    }
    ImGui::End();

    if (ImGui::Begin("Parallax Occlusion Mapping")) {
        for (int i = 0; i < scene.getEntities().size(); ++i) {
            BZ::Material &mat = scene.getEntities()[i].mesh.getMaterial();
            float scale = mat.getParallaxOcclusionScale();
            ImGui::PushID(i);
            if (ImGui::DragFloat("Scale", &scale, 0.001f, 0.0f, 2.0f)) {
                mat.setParallaxOcclusionScale(scale);
            }
            ImGui::PopID();
        }
    }
    ImGui::End();
}



BZ::Application* createApplication() {
    return new Sandbox();
}