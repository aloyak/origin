#include "engine/components/parentEntityComponent.h"
#include "engine/scene/sceneManager.h"

#include <algorithm>
#include <nlohmann/json.hpp>

namespace {
    Vec3 safeDivide(const Vec3& numerator, const Vec3& denominator) {
        return Vec3(
            denominator.x != 0.0f ? numerator.x / denominator.x : 1.0f,
            denominator.y != 0.0f ? numerator.y / denominator.y : 1.0f,
            denominator.z != 0.0f ? numerator.z / denominator.z : 1.0f
        );
    }
}

ParentEntityComponent::LocalTransform ParentEntityComponent::computeLocalTransform(Entity* child) const {
    LocalTransform local;
    if (!entity || !child) return local;

    local.position = child->transform.position - entity->transform.position;

    Quat entityRotQuat = Quat::fromEulerAngles(entity->transform.rotation.x, entity->transform.rotation.y, entity->transform.rotation.z);
    Quat childRotQuat  = Quat::fromEulerAngles(child->transform.rotation.x, child->transform.rotation.y, child->transform.rotation.z);
    local.rotation = inverse(entityRotQuat) * childRotQuat;

    local.scale = safeDivide(child->transform.scale, entity->transform.scale);

    return local;
}

void ParentEntityComponent::addChild(Entity* child, bool keepWorldTransform) {
    if (!child) return;
    m_children.push_back(child);

    if (keepWorldTransform && entity) {
        m_localTransforms[child] = computeLocalTransform(child);
    } else {
        m_localTransforms[child] = LocalTransform{};
    }
}

void ParentEntityComponent::removeChild(Entity* child) {
    if (!child) return;
    m_children.erase(std::remove(m_children.begin(), m_children.end(), child), m_children.end());
    m_localTransforms.erase(child);
}

void ParentEntityComponent::clearChildren() {
    m_children.clear();
    m_localTransforms.clear();
}

void ParentEntityComponent::refreshLocalTransform(Entity* child) {
    if (!child || !entity) return;
    m_localTransforms[child] = computeLocalTransform(child);
}

void ParentEntityComponent::update(float dt) {
    if (!entity) return;

    Quat entityRotQuat = Quat::fromEulerAngles(entity->transform.rotation.x, entity->transform.rotation.y, entity->transform.rotation.z);

    for (auto* child : m_children) {
        if (!child) continue;

        auto it = m_localTransforms.find(child);
        if (it == m_localTransforms.end()) {
            it = m_localTransforms.emplace(child, computeLocalTransform(child)).first;
        }
        const LocalTransform& local = it->second;

        if (applyPosition)
            child->transform.position = entity->transform.position + local.position;

        if (applyRotation) {
            Quat globalRotationQuat = entityRotQuat * local.rotation;
            child->transform.rotation = toEulerAngles(globalRotationQuat);
        }

        if (applyScale)
            child->transform.scale = entity->transform.scale * local.scale;
    }
}

void ParentEntityComponent::serialize(nlohmann::json& j) const {
    j["type"] = "ParentEntityComponent";
    j["children"] = nlohmann::json::array();
    for (const auto& child : m_children) {
        if (!child) continue;

        auto it = m_localTransforms.find(child);
        nlohmann::json childJson;
        childJson["name"] = child->name;
        if (it != m_localTransforms.end()) {
            const LocalTransform& local = it->second;
            childJson["localPosition"] = { local.position.x, local.position.y, local.position.z };
            childJson["localScale"]    = { local.scale.x, local.scale.y, local.scale.z };
            Vec3 localRotEuler = toEulerAngles(local.rotation);
            childJson["localRotationEuler"] = { localRotEuler.x, localRotEuler.y, localRotEuler.z };
        }
        j["children"].push_back(childJson);
    }
}

void ParentEntityComponent::deserialize(const nlohmann::json& j) {
    if (!j.contains("children") || !j["children"].is_array()) return;

    m_children.clear();
    m_localTransforms.clear();

    for (const auto& entry : j["children"]) {
        std::string childName;
        if (entry.is_string()) {
            childName = entry.get<std::string>();
        } else if (entry.is_object() && entry.contains("name") && entry["name"].is_string()) {
            childName = entry["name"].get<std::string>();
        } else {
            continue;
        }

        Entity* child = m_sceneManager.getActiveScene()->getEntityByName(childName).get();
        if (!child) continue;

        m_children.push_back(child);

        if (entry.is_object() && entry.contains("localPosition") && entry.contains("localScale")) {
            LocalTransform local;
            local.position = Vec3(entry["localPosition"][0], entry["localPosition"][1], entry["localPosition"][2]);
            local.scale    = Vec3(entry["localScale"][0], entry["localScale"][1], entry["localScale"][2]);
            if (entry.contains("localRotationEuler")) {
                Vec3 rotEuler(entry["localRotationEuler"][0], entry["localRotationEuler"][1], entry["localRotationEuler"][2]);
                local.rotation = Quat::fromEulerAngles(rotEuler.x, rotEuler.y, rotEuler.z);
            }
            m_localTransforms[child] = local;
        } else {
            m_localTransforms[child] = computeLocalTransform(child);
        }
    }
}