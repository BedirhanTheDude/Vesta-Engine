#include <property/PropertySerializer.h>

#include <glm/glm.hpp>
#include <persistance/Archive.h>

namespace PropertySerializer {
    void serialize(const PropertyHolder& holder, Archive& arch) {
        for (const auto& [idx, prop] : holder.getProperties()) {
            switch (prop.type) {
            case PropertyType::Bool:
                arch.set(prop.name, *static_cast<bool*>(prop.ptr));
                break;
            case PropertyType::Char:
                arch.set(prop.name, std::string(1, *static_cast<char*>(prop.ptr)));
                break;
            case PropertyType::Int:
                arch.set(prop.name, *static_cast<int*>(prop.ptr));
                break;
            case PropertyType::Float:
                arch.set(prop.name, *static_cast<float*>(prop.ptr));
                break;
            case PropertyType::Double:
                arch.set(prop.name, *static_cast<double*>(prop.ptr));
                break;
            case PropertyType::String:
                arch.set(prop.name, *static_cast<std::string*>(prop.ptr));
                break;
            case PropertyType::Vec2: {
                arch.set(prop.name, *static_cast<glm::vec2*>(prop.ptr));
                break;
            }
            case PropertyType::Vec3: {
                arch.set(prop.name, *static_cast<glm::vec3*>(prop.ptr));
                break;
            }
            case PropertyType::Vec4: {
                arch.set(prop.name, *static_cast<glm::vec4*>(prop.ptr));
                break;
            }
            case PropertyType::Mat3: {
                arch.set(prop.name, *static_cast<glm::mat3*>(prop.ptr));
                break;
            }
            case PropertyType::Mat4: {
                arch.set(prop.name, *static_cast<glm::mat4*>(prop.ptr));
                break;
            }
            }
        }
    }

    void deserialize(PropertyHolder& holder, const Archive& arch) {
        for (auto& [idx, prop] : holder.getProperties()) {
            if (!arch.has(prop.name)) continue;
            switch (prop.type) {
            case PropertyType::Bool:
                arch.get(prop.name, *static_cast<bool*>(prop.ptr));
                break;
            case PropertyType::Char: {
                std::string tmp;
                if (arch.get(prop.name, tmp) && !tmp.empty())
                    *static_cast<char*>(prop.ptr) = tmp[0];
                break;
            }
            case PropertyType::Int:
                arch.get(prop.name, *static_cast<int*>(prop.ptr));
                break;
            case PropertyType::Float:
                arch.get(prop.name, *static_cast<float*>(prop.ptr));
                break;
            case PropertyType::Double:
                arch.get(prop.name, *static_cast<double*>(prop.ptr));
                break;
            case PropertyType::String:
                arch.get(prop.name, *static_cast<std::string*>(prop.ptr));
                break;
            case PropertyType::Vec2: {
                arch.get(prop.name, *static_cast<glm::vec2*>(prop.ptr));
                break;
            }
            case PropertyType::Vec3: {
                arch.get(prop.name, *static_cast<glm::vec3*>(prop.ptr));
                break;
            }
            case PropertyType::Vec4: {
                arch.get(prop.name, *static_cast<glm::vec4*>(prop.ptr));
                break;
            }
            case PropertyType::Mat3: {
                arch.get(prop.name, *static_cast<glm::mat3*>(prop.ptr));
                break;
            }
            case PropertyType::Mat4: {
                arch.get(prop.name, *static_cast<glm::mat4*>(prop.ptr));
                break;
            }
            }
        }
    }
}