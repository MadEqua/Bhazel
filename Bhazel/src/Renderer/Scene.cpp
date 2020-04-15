#include "bzpch.h"

#include "Scene.h"
#include "Renderer.h"


namespace BZ {

    Entity::Entity(Mesh &mesh, Transform &transform) :
        mesh(mesh), transform(transform) {
    }

    Scene::Scene() {
        descriptorSet = Renderer::createSceneDescriptorSet();
    }

    Scene::Scene(Camera &camera) :
        camera(&camera) {
        descriptorSet = Renderer::createSceneDescriptorSet();
    }

    void Scene::addEntity(Mesh &mesh, Transform &transform) {
        entities.push_back({ mesh, transform });
    }

    void Scene::addDirectionalLight(DirectionalLight &light) {
        lights.push_back(light);

        constexpr uint32 SIZE = 1024;
        auto shadowMap = Texture2D::createRenderTarget(SIZE, SIZE, TextureFormat::D24S8); //TODO: depth only format

        Framebuffer::Builder framebufferBuilder;
        AttachmentDescription depthStencilAttachmentDesc;
        depthStencilAttachmentDesc.format = shadowMap->getFormat().format;
        depthStencilAttachmentDesc.samples = 1;
        depthStencilAttachmentDesc.loadOperatorColorAndDepth = LoadOperation::DontCare;
        depthStencilAttachmentDesc.storeOperatorColorAndDepth = StoreOperation::Store;
        depthStencilAttachmentDesc.loadOperatorStencil = LoadOperation::DontCare;
        depthStencilAttachmentDesc.storeOperatorStencil = StoreOperation::Store;
        depthStencilAttachmentDesc.initialLayout = TextureLayout::Undefined;
        depthStencilAttachmentDesc.finalLayout = TextureLayout::DepthStencilAttachmentOptimal;
        depthStencilAttachmentDesc.clearValues.floating.x = 1.0f;
        depthStencilAttachmentDesc.clearValues.integer.y = 0;
        framebufferBuilder.addDepthStencilAttachment(depthStencilAttachmentDesc, TextureView::create(shadowMap));
        shadowMapFramebuffers.push_back(framebufferBuilder.build());
    }

    void Scene::enableSkyBox(const char *albedoBasePath, const char *albedoFileNames[6],
                             const char *irradianceMapBasePath, const char *irradianceMapFileNames[6],
                             const char *radianceMapBasePath, const char *radianceMapFileNames[6], uint32 radianceMipmapCount) {
        
        auto albedoTexRef = TextureCube::create(albedoBasePath, albedoFileNames, TextureFormat::R8G8B8A8_SRGB, MipmapData::Options::Generate);
        skyBox.mesh = Mesh::createUnitCubeInsides(Material(albedoTexRef));

        auto irradianceMapTexRef = TextureCube::create(irradianceMapBasePath, irradianceMapFileNames, TextureFormat::R8G8B8A8_SRGB, MipmapData::Options::Generate);
        skyBox.irradianceMapView = TextureView::create(irradianceMapTexRef);

        auto radianceMapTexRef = TextureCube::create(radianceMapBasePath, radianceMapFileNames, TextureFormat::R8G8B8A8_SRGB, { MipmapData::Options::Load, radianceMipmapCount });
        skyBox.radianceMapView = TextureView::create(radianceMapTexRef);

        descriptorSet->setCombinedTextureSampler(skyBox.irradianceMapView, Renderer::getDefaultSampler(), 1);
        descriptorSet->setCombinedTextureSampler(skyBox.radianceMapView, Renderer::getDefaultSampler(), 2);
    }

    void Scene::setCamera(Camera &camera) {
        this->camera = &camera;
    }
}