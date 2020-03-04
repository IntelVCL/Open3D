// ----------------------------------------------------------------------------
// -                        Open3D: www.open3d.org                            -
// ----------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2019 www.open3d.org
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------------------------------------------------------

#pragma once

#include "Open3D/Visualization/Rendering/Scene.h"

#include <memory>
#include <unordered_map>

#include <filament/utils/Entity.h>
#include <filament/utils/EntityInstance.h>

namespace filament {
class Engine;
class IndirectLight;
class Renderer;
class Scene;
class Skybox;
class TransformManager;
class VertexBuffer;
}  // namespace filament

namespace open3d {
namespace visualization {

class FilamentResourceManager;
class FilamentView;

class FilamentScene : public Scene {
public:
    FilamentScene(filament::Engine& engine,
                  FilamentResourceManager& resourceManager);
    ~FilamentScene() override;

    // All views above first will discard
    // only depth and stencil buffers by default
    ViewHandle AddView(std::int32_t x,
                       std::int32_t y,
                       std::uint32_t w,
                       std::uint32_t h) override;

    View* GetView(const ViewHandle& viewId) const override;
    void SetViewActive(const ViewHandle& viewId, bool isActive) override;
    void RemoveView(const ViewHandle& viewId) override;

    GeometryHandle AddGeometry(const geometry::Geometry3D& geometry) override;
    GeometryHandle AddGeometry(
            const geometry::Geometry3D& geometry,
            const MaterialInstanceHandle& materialId) override;
    GeometryHandle AddGeometry(const geometry::Geometry3D& geometry,
                               const MaterialInstanceHandle& materialId,
                               const std::string& name) override;
    std::vector<GeometryHandle> FindGeometryByName(
            const std::string& name) override;
    void AssignMaterial(const GeometryHandle& geometryId,
                        const MaterialInstanceHandle& materialId) override;
    MaterialInstanceHandle GetMaterial(
            const GeometryHandle& geometryId) const override;
    void RemoveGeometry(const GeometryHandle& geometryId) override;

    LightHandle AddLight(const LightDescription& descr) override;
    void SetLightIntensity(const LightHandle& id, float intensity) override;
    void SetLightColor(const LightHandle& id,
                       const Eigen::Vector3f& color) override;
    void SetLightDirection(const LightHandle& id,
                           const Eigen::Vector3f& pos) override;
    void SetLightPosition(const LightHandle& id,
                          const Eigen::Vector3f& pos) override;
    void SetLightFalloff(const LightHandle& id, float falloff) override;
    void RemoveLight(const LightHandle& id) override;

    void SetIndirectLight(const IndirectLightHandle& id) override;
    void SetIndirectLightIntensity(float intensity) override;
    float GetIndirectLightIntensity() const override;
    void SetIndirectLightRotation(const Transform& rotation) override;
    Transform GetIndirectLightRotation() const override;

    void SetSkybox(const SkyboxHandle& id) override;

    void SetEntityEnabled(const REHandle_abstract& entityId,
                          bool enabled) override;
    void SetEntityTransform(const REHandle_abstract& entityId,
                            const Transform& transform) override;
    Transform GetEntityTransform(const REHandle_abstract& entityId) override;

    std::pair<Eigen::Vector3f, Eigen::Vector3f> GetEntityBoundingBox(
            const REHandle_abstract& entityId) override;
    std::pair<Eigen::Vector3f, float> GetEntityBoundingSphere(
            const REHandle_abstract& entityId) override;

    void Draw(filament::Renderer& renderer);

    filament::Scene* GetNativeScene() const { return scene_; }

private:
    friend class FilamentView;

    struct SceneEntity {
        struct Details {
            utils::Entity self;
            EntityType type;
            VertexBufferHandle vb;
            IndexBufferHandle ib;

            bool IsValid() const { return !self.isNull(); }
            void ReleaseResources(filament::Engine& engine,
                                  FilamentResourceManager& manager);
        } info;

        // We can disable entities removing them from scene, but not
        // deallocating
        bool enabled = true;
        MaterialInstanceHandle material;
        TextureHandle texture;  // if none, default is used
        // Used for relocating transform to center of mass
        utils::Entity parent;
        std::string name;

        bool IsValid() const { return info.IsValid(); }
        void ReleaseResources(filament::Engine& engine,
                              FilamentResourceManager& manager);
    };

    struct ViewContainer {
        std::unique_ptr<FilamentView> view;
        bool isActive = true;
    };

    utils::EntityInstance<filament::TransformManager>
    GetEntityTransformInstance(const REHandle_abstract& id);
    void RemoveEntity(REHandle_abstract id);

    filament::Scene* scene_ = nullptr;

    filament::Engine& engine_;
    FilamentResourceManager& resourceManager_;

    std::unordered_map<REHandle_abstract, ViewContainer> views_;
    std::unordered_map<REHandle_abstract, SceneEntity> entities_;
    std::weak_ptr<filament::IndirectLight> wIndirectLight_;
    std::weak_ptr<filament::Skybox> wSkybox_;
};

}  // namespace visualization
}  // namespace open3d
