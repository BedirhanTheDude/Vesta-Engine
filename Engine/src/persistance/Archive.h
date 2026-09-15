#pragma once

#include <string>
#include <cstdint>
#include <cstddef>
#include <glm/fwd.hpp>

// binary serialization interface
// 
// On-disk layout:
//   File   : [magic 'VREA'][version u8][endianFlag u8][root object]
//   Value  : [tag u8][payload]
//   Scalar : fixed-size payload (int=4, float=4, vec3=12, ...)
//   String : [len u32][bytes]
//   Object : [byteLength u32][fieldCount u32]  then fieldCount * Entry
//   Array  : [byteLength u32][elemCount u32]   then elemCount  * Value (no keys)
//   Entry  : [keyLen u32][keyBytes][value]
//
// objects/arrays are length-prefixed so unknown records can be skipped wholesale.
// reading is parse-on-load: the whole tree is materialised in memory up front.

class Archive {
public:
    Archive();
    ~Archive();

    Archive(Archive&&) noexcept;
    Archive& operator=(Archive&&) noexcept;
    Archive(const Archive&) = delete;
    Archive& operator=(const Archive&) = delete;

    // file io
    bool saveToFile(const std::string& path) const;
    bool loadFromFile(const std::string& path);

    // raw buffer
    bool serializeToBuffer(std::string& outBytes) const;
    bool parseFromBuffer(const std::string& bytes);

    // write scalars
    void set(const std::string& key, int v);
    void set(const std::string& key, unsigned v);
    void set(const std::string& key, float v);
    void set(const std::string& key, double v);
    void set(const std::string& key, bool v);
    void set(const std::string& key, const std::string& v);
    void set(const std::string& key, const glm::vec2& v);
    void set(const std::string& key, const glm::vec3& v);
    void set(const std::string& key, const glm::vec4& v);
    void set(const std::string& key, const glm::quat& v);
    void set(const std::string& key, const glm::mat3& v);
    void set(const std::string& key, const glm::mat4& v);

    // write nested
    void set(const std::string& key, Archive&& child);     // set an object
    void append(const std::string& key, Archive&& child);  // push onto array

    // read scalar
    bool get(const std::string& key, int& out) const;
    bool get(const std::string& key, unsigned& out) const;
    bool get(const std::string& key, float& out) const;
    bool get(const std::string& key, double& out) const;
    bool get(const std::string& key, bool& out) const;
    bool get(const std::string& key, std::string& out) const;
    bool get(const std::string& key, glm::vec2& out) const;
    bool get(const std::string& key, glm::vec3& out) const;
    bool get(const std::string& key, glm::vec4& out) const;
    bool get(const std::string& key, glm::quat& out) const;
    bool get(const std::string& key, glm::mat3& out) const;
    bool get(const std::string& key, glm::mat4& out) const;

    // read nested
    bool    has(const std::string& key) const;
    Archive get(const std::string& key) const;              // object copy
    size_t  size(const std::string& key) const;             // array length, 0 is not exists
    Archive at(const std::string& key, size_t index) const; // array element copy

private:
    // pointer implementation
    struct Impl;
    Impl* impl_;

    explicit Archive(Impl* adopt);
};