#pragma once

#include "GameX/renderer/camera.h"
#include "GameX/renderer/entity.h"
#include "GameX/renderer/light.h"
#include "GameX/renderer/render_pipeline.h"

namespace GameX::Graphics {

struct SceneSettings {
  int max_objects = 1000;
  int max_ambient_light = 100;
  int max_cameras = 100;
};

GAMEX_CLASS(Scene) : public grassland::vulkan::DynamicObject {
 public:
  Scene(class Renderer * renderer, const SceneSettings &settings = {});

  ~Scene();

  class Renderer *Renderer() {
    return renderer_;
  }

  grassland::vulkan::DescriptorPool *DescriptorPool() {
    return descriptor_pool_.get();
  }

  template <class... Args>
  [[nodiscard]] UCamera CreateCamera(Args && ...args) {
    auto camera = std::make_unique<Camera>(this, std::forward<Args>(args)...);
    cameras_.insert(camera.get());
    return camera;
  }

  template <class... Args>
  [[nodiscard]] UEntity CreateEntity(Args && ...args) {
    auto entity = std::make_unique<Entity>(this, std::forward<Args>(args)...);
    entities_.insert(entity.get());
    return entity;
  }

  template <class LightType, class... Args>
  [[nodiscard]] std::unique_ptr<LightType> CreateLight(Args && ...args) {
    auto light = std::make_unique<LightType>(this, std::forward<Args>(args)...);
    lights_.insert(light.get());
    return light;
  }

  void DestroyCamera(Camera * camera) {
    cameras_.erase(camera);
  }

  void DestroyEntity(Entity * entity) {
    entities_.erase(entity);
  }

  void DestroyLight(Light * light) {
    lights_.erase(light);
  }

  const std::set<Entity *> &Entities() const {
    return entities_;
  }

  const std::set<Light *> &Lights() const {
    return lights_;
  }

  bool SyncData(VkCommandBuffer cmd_buffer, uint32_t frame_index) override;

 private:
  struct EnvmapData {
    float offset;
    float exposure;
  };

  class Renderer *renderer_;

  std::unique_ptr<grassland::vulkan::DescriptorPool> descriptor_pool_;
  std::set<Camera *> cameras_;
  std::set<Entity *> entities_;
  std::set<Light *> lights_;

  Image *envmap_image_{};
  grassland::vulkan::Sampler *envmap_sampler_{};
  std::unique_ptr<grassland::vulkan::DynamicBuffer<EnvmapData>>
      envmap_data_buffer_;
  std::vector<uint64_t> envmap_versions_;
  uint64_t staging_envmap_version_{};
  std::vector<std::unique_ptr<grassland::vulkan::DescriptorSet>>
      envmap_descriptor_sets_;
};

grassland::vulkan::DescriptorSetLayout *EnvmapDescriptorSetLayout(
    class Renderer *renderer);

}  // namespace GameX::Graphics
