#include "SandboxApp.h"

#include <glm/gtc/matrix_transform.hpp>
#include <ImGui/imgui.h>


ExampleLayer::ExampleLayer() : Layer("Example"), camera(-1.6f, 1.6f, -0.9f, 0.9f), cameraPos(0.0f), cameraRot(0.0f) {

    const char * glVS = R"(
            #version 430 core
            layout(location = 0) in vec3 pos;
            layout(location = 1) in vec3 col;
            layout(location = 2) in vec2 texCoord;
            
            out vec3 vCol;
            out vec2 vTexCoord;

            uniform mat4 viewProjectionMatrix;
            uniform mat4 modelMatrix;

            void main() {
                vCol = col;
                vTexCoord = texCoord;
                //gl_Position = viewProjectionMatrix * modelMatrix * vec4(pos, 1.0);
                gl_Position = vec4(pos, 1.0);
            }
        )";
    const char * glFS = R"(
            #version 430 core
            layout(location = 0) out vec4 col;

            layout(location = 0) uniform sampler2D colorTexture;

            in vec3 vCol;
            in vec2 vTexCoord;
            
            void main() {
                vec4 texCol = texture(colorTexture, vTexCoord);
                //col = vec4(texCol.rgb, 1.0);
                col = vec4(vCol, 1.0);
            }
        )";

    const char* d3dVS = R"(
            struct VsOut {
                float3 col : COLOR;
                float4 pos : SV_POSITION;
            };

            VsOut main(float3 pos : POSITION, float3 col : COLOR) {
                VsOut res;
                res.pos = float4(pos, 1.0);
                res.col = col;
                return res;
            }
        )";

    const char* d3dPS = R"(
            struct PsIn {
                float3 col : COLOR;
            };

            float4 main(PsIn input) : SV_TARGET {
                return float4(input.col, 1.0);
            }
        )";

    shader = BZ::RendererAPI::getAPI() == BZ::RendererAPI::API::OpenGL ? BZ::Shader::create(glVS, glFS) : BZ::Shader::create(d3dVS, d3dPS);
    //texture = BZ::Texture2D::create("test.jpg");

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 0.0f,

        0.5f, -0.5f, 0.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 0.0f,

        0.5f, 0.5f, 0.0f,
        0.0f, 0.0f, 1.0f,
        1.0f, 1.0f,

        -0.5f, 0.5f, 0.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 1.0f,
    };

    BZ::BufferLayout layout = {
        {BZ::ShaderDataType::Vec3, "POSITION"},
        {BZ::ShaderDataType::Vec3, "COLOR"},
        {BZ::ShaderDataType::Vec2, "TEXCOORD"}
    };
    vertexBuffer = BZ::VertexBuffer::create(vertices, sizeof(vertices), layout);
    vertexBuffer->bind();

    uint32 indices[] = {0, 1, 2, 0, 2, 3};
    indexBuffer = BZ::IndexBuffer::create(indices, sizeof(indices) / sizeof(uint32));
    indexBuffer->bind();

    inputDescription = BZ::InputDescription::create();
    inputDescription->addVertexBuffer(vertexBuffer, shader);
    inputDescription->setIndexBuffer(indexBuffer);

    BZ::RenderCommand::setClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
}

void ExampleLayer::onUpdate(BZ::Timestep timestep) {
    const float CAMERA_MOVE_SPEED = 3.0f * timestep;
    const float CAMERA_ROT_SPEED = 180.0f * timestep;
    const float TRI_MOVE_SPEED = 3.0f * timestep;
    
    if(BZ::Input::isKeyPressed(BZ_KEY_A)) cameraPos.x -= CAMERA_MOVE_SPEED;
    else if(BZ::Input::isKeyPressed(BZ_KEY_D)) cameraPos.x += CAMERA_MOVE_SPEED;
    
    if(BZ::Input::isKeyPressed(BZ_KEY_W)) cameraPos.y += CAMERA_MOVE_SPEED;
    else if(BZ::Input::isKeyPressed(BZ_KEY_S)) cameraPos.y -= CAMERA_MOVE_SPEED;

    if(BZ::Input::isKeyPressed(BZ_KEY_Q)) cameraRot -= CAMERA_ROT_SPEED;
    else if(BZ::Input::isKeyPressed(BZ_KEY_E)) cameraRot += CAMERA_ROT_SPEED;

    camera.setPosition(cameraPos);
    camera.setRotation(cameraRot);

    BZ::RenderCommand::clearColorAndDepthStencilBuffers();

    BZ::Renderer::beginScene(camera);

    /*glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
    for(int i = 0; i < 20; ++i) {
        for(int j = 0; j < 20; ++j) {
            glm::vec3 pos(i * 0.11f, j * 0.11f, 0);
            glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), pos) * scaleMat;
            BZ::Renderer::submit(shader, inputDescription, modelMatrix);
        }
    }*/

    //texture->bind(0);
    BZ::Renderer::submit(shader, inputDescription);
    BZ::Renderer::endScene();
}

void ExampleLayer::onEvent(BZ::Event &event) {
    BZ::EventDispatcher dispatcher(event);
    dispatcher.dispatch<BZ::KeyPressedEvent>(BZ_BIND_EVENT_FN(ExampleLayer::onKeyPressedEvent));
}

void ExampleLayer::onImGuiRender() {
    ImGui::Begin("Test");
    ImGui::LabelText("Hello!", "");
    ImGui::End();
}

bool ExampleLayer::onKeyPressedEvent(BZ::KeyPressedEvent &event) {
    //BZ_LOG_TRACE("KeyPressed: {0}", event);
    return false;
}



BZ::Application* BZ::createApplication() {
    return new Sandbox();
}

#include <Bhazel/EntryPoint.h>