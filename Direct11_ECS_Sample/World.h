#pragma once

#include "Entity.h"
#include "Component.h"
#include "System.h"

#include <any>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

template <typename T>
class ComponentStore
{
public:
    bool Has(Entity entity) const
    {
        return components_.contains(entity);
    }

    T& Add(Entity entity, const T& value = {})
    {
        return components_.emplace(entity, value).first->second;
    }

    void Remove(Entity entity)
    {
        components_.erase(entity);
    }

    T* TryGet(Entity entity)
    {
        auto it = components_.find(entity);
        return it != components_.end() ? &it->second : nullptr;
    }

    std::unordered_map<Entity, T>& All()
    {
        return components_;
    }

    const std::unordered_map<Entity, T>& All() const
    {
        return components_;
    }

private:
    std::unordered_map<Entity, T> components_;
};

class World
{
public:
    Entity CreateEntity();
    void DestroyEntity(Entity entity);

    template <typename T>
    T& AddComponent(Entity entity, const T& value = {})
    {
        return GetStore<T>().Add(entity, value);
    }

    template <typename T>
    void RemoveComponent(Entity entity)
    {
        GetStore<T>().Remove(entity);
    }

    template <typename T>
    T* GetComponent(Entity entity)
    {
        return GetStore<T>().TryGet(entity);
    }

    template <typename T>
    bool HasComponent(Entity entity) const
    {
        auto it = stores_.find(std::type_index(typeid(T)));
        if (it == stores_.end())
        {
            return false;
        }
        return std::any_cast<const ComponentStore<T>&>(it->second).Has(entity);
    }

    template <typename T>
    ComponentStore<T>& GetStore()
    {
        auto key = std::type_index(typeid(T));
        auto it = stores_.find(key);
        if (it == stores_.end())
        {
            it = stores_.emplace(key, std::make_any<ComponentStore<T>>()).first;
        }
        return std::any_cast<ComponentStore<T>&>(it->second);
    }

    void AddSystem(std::shared_ptr<ISystem> system);
    void UpdateSystems(float deltaTime);
    void ClearEntities();

private:
    Entity next_ = 1;
    std::vector<Entity> freeList_;
    std::unordered_map<std::type_index, std::any> stores_;
    std::vector<std::shared_ptr<ISystem>> systems_;
};
