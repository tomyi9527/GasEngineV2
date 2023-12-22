#pragma once
#include <array>
#include <memory>
#include <set>
#include <string>

class Entity;

enum kComponentType {
    kAnimator,
    kMeshFilter,
    kMeshRenderer,
    kCamera,
    kPunctualLight,
    kEnvironmentalLight,
    kResource,

    kDirectionalLight,
    kPointLight,
    kSpotLight,

    kComponentTypeCount
};

class Component {
 public:
    virtual ~Component() {}
    kComponentType GetType() const { return type_; }
    std::string GetTypeStr() const;
    static int GetTypeByStr(std::string name);
    void SetParent(const std::shared_ptr<Entity>& entity) { parent_entity_ = entity; }
    int64_t GetID() const { return id; }
    void SetID(int64_t in_id) { id = in_id; }

 protected:
    Component(kComponentType type) : type_(type), parent_entity_(nullptr) {}
    void SetType(kComponentType type) { type_ = type; }
    kComponentType type_;
    int64_t id = -1;
    std::shared_ptr<Entity> parent_entity_;
};

class ComponentFactory {
 public:
    static ComponentFactory& Instance() {
        static ComponentFactory instance;
        return instance;
    }
    // std::shared_ptr<Component> Create(const std::string& name, uint64_t uid);
    std::shared_ptr<Component> Create(kComponentType type);

 private:
    ComponentFactory();
    std::array<int, kComponentTypeCount> ComponentConstructors;
    // std::set<std::shared_ptr<Component>> used_components;
};

using pComponent = std::shared_ptr<Component>;