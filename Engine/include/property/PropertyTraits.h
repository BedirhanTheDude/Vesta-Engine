#pragma once

#include <property/Property.h>
#include <glm/glm.hpp>
#include <string>

template<typename T> struct PropertyTypeOf;
template<> struct PropertyTypeOf<bool> { static constexpr PropertyType value = PropertyType::Bool; };
template<> struct PropertyTypeOf<char> { static constexpr PropertyType value = PropertyType::Char; };
template<> struct PropertyTypeOf<int> { static constexpr PropertyType value = PropertyType::Int; };
template<> struct PropertyTypeOf<float> { static constexpr PropertyType value = PropertyType::Float; };
template<> struct PropertyTypeOf<double> { static constexpr PropertyType value = PropertyType::Double; };
template<> struct PropertyTypeOf<std::string> { static constexpr PropertyType value = PropertyType::String; };
template<> struct PropertyTypeOf<glm::vec2> { static constexpr PropertyType value = PropertyType::Vec2; };
template<> struct PropertyTypeOf<glm::vec3> { static constexpr PropertyType value = PropertyType::Vec3; };
template<> struct PropertyTypeOf<glm::vec4> { static constexpr PropertyType value = PropertyType::Vec4; };
template<> struct PropertyTypeOf<glm::mat3> { static constexpr PropertyType value = PropertyType::Mat3; };
template<> struct PropertyTypeOf<glm::mat4> { static constexpr PropertyType value = PropertyType::Mat4; };