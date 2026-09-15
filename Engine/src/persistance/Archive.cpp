#include <persistance/Archive.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>
#include <map>
#include <memory>
#include <fstream>
#include <cstring>
#include <utility>

namespace {

    enum class Tag : uint8_t {
        Null = 0,
        Int = 1,
        UInt = 2,
        Float = 3,
        Double = 4,
        Bool = 5,
        String = 6,
        Vec2 = 7,
        Vec3 = 8,
        Vec4 = 9,
        Quat = 10,
        Object = 11,
        Array = 12,
        Mat3 = 13,
        Mat4 = 14,
    };

    constexpr char     MAGIC[4] = { 'V', 'E', 'R', 'O' };
    constexpr uint8_t  VERSION = 1;
    constexpr uint8_t  ENDIAN_LITTLE = 0; // native endian

    // A parsed value. Scalars store their bytes inline in `scalar`; Object/Array
    // recurse via children. Inline string keys live on the parent's entry map.
    struct Node {
        Tag tag = Tag::Null;

        // scalar storage (interpreted per tag)
        std::vector<uint8_t> scalar;

        // object: key -> child   (insertion order kept separately for stable output)
        std::map<std::string, std::shared_ptr<Node>> fields;
        std::vector<std::string> fieldOrder;

        // array: ordered children
        std::vector<std::shared_ptr<Node>> elements;
    };

    // ---- little binary write helpers (native endian) ----
    template <typename T>
    void putPOD(std::vector<uint8_t>& buf, const T& v) {
        const auto* p = reinterpret_cast<const uint8_t*>(&v);
        buf.insert(buf.end(), p, p + sizeof(T));
    }

    void putU32(std::vector<uint8_t>& buf, uint32_t v) { putPOD(buf, v); }
    void putU8(std::vector<uint8_t>& buf, uint8_t v) { buf.push_back(v); }

    void putBytes(std::vector<uint8_t>& buf, const void* data, size_t n) {
        const auto* p = static_cast<const uint8_t*>(data);
        buf.insert(buf.end(), p, p + n);
    }

    // ---- little binary read helpers (bounds-checked) ----
    struct Reader {
        const uint8_t* p;
        const uint8_t* end;
        bool ok = true;

        bool ensure(size_t n) {
            if (!ok || size_t(end - p) < n) { ok = false; return false; }
            return true;
        }
        template <typename T> T getPOD() {
            T v{};
            if (ensure(sizeof(T))) { std::memcpy(&v, p, sizeof(T)); p += sizeof(T); }
            return v;
        }
        uint8_t  getU8() { return getPOD<uint8_t>(); }
        uint32_t getU32() { return getPOD<uint32_t>(); }
    };

    // ---------------------------------------------------------------------------
    // Serialize a Node tree -> bytes
    // ---------------------------------------------------------------------------
    void writeValue(std::vector<uint8_t>& buf, const Node& n);

    void writeObjectBody(std::vector<uint8_t>& body, const Node& n) {
        putU32(body, static_cast<uint32_t>(n.fieldOrder.size()));
        for (const auto& key : n.fieldOrder) {
            auto it = n.fields.find(key);
            if (it == n.fields.end()) continue;
            putU32(body, static_cast<uint32_t>(key.size()));
            putBytes(body, key.data(), key.size());
            writeValue(body, *it->second);
        }
    }

    void writeArrayBody(std::vector<uint8_t>& body, const Node& n) {
        putU32(body, static_cast<uint32_t>(n.elements.size()));
        for (const auto& e : n.elements)
            writeValue(body, *e);
    }

    void writeValue(std::vector<uint8_t>& buf, const Node& n) {
        putU8(buf, static_cast<uint8_t>(n.tag));

        switch (n.tag) {
        case Tag::Null:
            break;
        case Tag::Int:
        case Tag::UInt:
        case Tag::Float:
        case Tag::Double:
        case Tag::Bool:
        case Tag::Vec2:
        case Tag::Vec3:
        case Tag::Vec4:
        case Tag::Quat:
        case Tag::Mat3:
        case Tag::Mat4:
            // fixed-size scalar payload, already laid out in scalar bytes
            putBytes(buf, n.scalar.data(), n.scalar.size());
            break;
        case Tag::String: {
            putU32(buf, static_cast<uint32_t>(n.scalar.size()));
            putBytes(buf, n.scalar.data(), n.scalar.size());
            break;
        }
        case Tag::Object: {
            std::vector<uint8_t> body;
            writeObjectBody(body, n);
            putU32(buf, static_cast<uint32_t>(body.size()));
            putBytes(buf, body.data(), body.size());
            break;
        }
        case Tag::Array: {
            std::vector<uint8_t> body;
            writeArrayBody(body, n);
            putU32(buf, static_cast<uint32_t>(body.size()));
            putBytes(buf, body.data(), body.size());
            break;
        }
        }
    }

    // ---------------------------------------------------------------------------
    // Parse bytes -> Node tree
    // ---------------------------------------------------------------------------
    std::shared_ptr<Node> readValue(Reader& r);

    void readObjectBody(Reader& r, Node& n) {
        uint32_t count = r.getU32();
        for (uint32_t i = 0; i < count && r.ok; i++) {
            uint32_t keyLen = r.getU32();
            if (!r.ensure(keyLen)) return;
            std::string key(reinterpret_cast<const char*>(r.p), keyLen);
            r.p += keyLen;
            auto child = readValue(r);
            if (!r.ok) return;
            n.fields[key] = child;
            n.fieldOrder.push_back(key);
        }
    }

    void readArrayBody(Reader& r, Node& n) {
        uint32_t count = r.getU32();
        for (uint32_t i = 0; i < count && r.ok; i++) {
            auto child = readValue(r);
            if (!r.ok) return;
            n.elements.push_back(child);
        }
    }

    size_t scalarSize(Tag t) {
        switch (t) {
        case Tag::Int:    return 4;
        case Tag::UInt:   return 4;
        case Tag::Float:  return 4;
        case Tag::Double: return 8;
        case Tag::Bool:   return 1;
        case Tag::Vec2:   return 8;
        case Tag::Vec3:   return 12;
        case Tag::Vec4:   return 16;
        case Tag::Quat:   return 16;
        case Tag::Mat3:   return 36;
        case Tag::Mat4:   return 64;
        default:          return 0;
        }
    }

    std::shared_ptr<Node> readValue(Reader& r) {
        auto n = std::make_shared<Node>();
        n->tag = static_cast<Tag>(r.getU8());
        if (!r.ok) return n;

        switch (n->tag) {
        case Tag::Null:
            break;
        case Tag::Int:
        case Tag::UInt:
        case Tag::Float:
        case Tag::Double:
        case Tag::Bool:
        case Tag::Vec2:
        case Tag::Vec3:
        case Tag::Vec4:
        case Tag::Quat:
        case Tag::Mat3:
        case Tag::Mat4: {
            size_t sz = scalarSize(n->tag);
            if (!r.ensure(sz)) break;
            n->scalar.assign(r.p, r.p + sz);
            r.p += sz;
            break;
        }
        case Tag::String: {
            uint32_t len = r.getU32();
            if (!r.ensure(len)) break;
            n->scalar.assign(r.p, r.p + len);
            r.p += len;
            break;
        }
        case Tag::Object: {
            uint32_t bodyLen = r.getU32();
            if (!r.ensure(bodyLen)) break;
            Reader sub{ r.p, r.p + bodyLen, true };
            readObjectBody(sub, *n);
            r.p += bodyLen;             // skip wholesale regardless of sub parse
            if (!sub.ok) r.ok = false;
            break;
        }
        case Tag::Array: {
            uint32_t bodyLen = r.getU32();
            if (!r.ensure(bodyLen)) break;
            Reader sub{ r.p, r.p + bodyLen, true };
            readArrayBody(sub, *n);
            r.p += bodyLen;
            if (!sub.ok) r.ok = false;
            break;
        }
        default:
            r.ok = false;               // unknown tag
            break;
        }
        return n;
    }

    // ---- scalar node builders ----
    template <typename T>
    std::shared_ptr<Node> makeScalar(Tag tag, const T& v) {
        auto n = std::make_shared<Node>();
        n->tag = tag;
        n->scalar.resize(sizeof(T));
        std::memcpy(n->scalar.data(), &v, sizeof(T));
        return n;
    }

    std::shared_ptr<Node> makeString(const std::string& s) {
        auto n = std::make_shared<Node>();
        n->tag = Tag::String;
        n->scalar.assign(s.begin(), s.end());
        return n;
    }

    // read a scalar node back into T (returns false on tag mismatch)
    template <typename T>
    bool readScalar(const Node* n, Tag expect, T& out) {
        if (!n || n->tag != expect || n->scalar.size() != sizeof(T)) return false;
        std::memcpy(&out, n->scalar.data(), sizeof(T));
        return true;
    }

} // anonymous namespace

// ---------------------------------------------------------------------------
// Impl
// ---------------------------------------------------------------------------
struct Archive::Impl {
    // Every archive's root is an object node.
    std::shared_ptr<Node> root = std::make_shared<Node>();
    Impl() { root->tag = Tag::Object; }

    Node* obj() { return root.get(); }
    const Node* obj() const { return root.get(); }

    void put(const std::string& key, std::shared_ptr<Node> child) {
        if (root->fields.find(key) == root->fields.end())
            root->fieldOrder.push_back(key);
        root->fields[key] = std::move(child);
    }

    const Node* find(const std::string& key) const {
        auto it = root->fields.find(key);
        return it == root->fields.end() ? nullptr : it->second.get();
    }
};

// ---------------------------------------------------------------------------
// ctor / dtor / move
// ---------------------------------------------------------------------------
Archive::Archive() : impl_(new Impl()) {}
Archive::~Archive() { delete impl_; }
Archive::Archive(Impl* adopt) : impl_(adopt) {}

Archive::Archive(Archive&& o) noexcept : impl_(o.impl_) { o.impl_ = nullptr; }
Archive& Archive::operator=(Archive&& o) noexcept {
    if (this != &o) { delete impl_; impl_ = o.impl_; o.impl_ = nullptr; }
    return *this;
}

// ---------------------------------------------------------------------------
// buffer io
// ---------------------------------------------------------------------------
bool Archive::serializeToBuffer(std::string& outBytes) const {
    std::vector<uint8_t> buf;
    putBytes(buf, MAGIC, 4);
    putU8(buf, VERSION);
    putU8(buf, ENDIAN_LITTLE);
    writeValue(buf, *impl_->root);
    outBytes.assign(reinterpret_cast<const char*>(buf.data()), buf.size());
    return true;
}

bool Archive::parseFromBuffer(const std::string& bytes) {
    Reader r{ reinterpret_cast<const uint8_t*>(bytes.data()),
              reinterpret_cast<const uint8_t*>(bytes.data()) + bytes.size(), true };

    if (!r.ensure(6)) return false;
    if (std::memcmp(r.p, MAGIC, 4) != 0) return false;
    r.p += 4;
    uint8_t version = r.getU8();
    uint8_t endian = r.getU8();
    (void)version; (void)endian;   // reserved for future use

    auto root = readValue(r);
    if (!r.ok || !root || root->tag != Tag::Object) return false;

    impl_->root = root;
    return true;
}

bool Archive::saveToFile(const std::string& path) const {
    std::string bytes;
    if (!serializeToBuffer(bytes)) return false;
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;
    f.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    return static_cast<bool>(f);
}

bool Archive::loadFromFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    std::string bytes((std::istreambuf_iterator<char>(f)),
        std::istreambuf_iterator<char>());
    return parseFromBuffer(bytes);
}

// ---------------------------------------------------------------------------
// write scalars
// ---------------------------------------------------------------------------
void Archive::set(const std::string& k, int v) { impl_->put(k, makeScalar(Tag::Int, v)); }
void Archive::set(const std::string& k, unsigned int v) { impl_->put(k, makeScalar(Tag::UInt, v)); }
void Archive::set(const std::string& k, float v) { impl_->put(k, makeScalar(Tag::Float, v)); }
void Archive::set(const std::string& k, double v) { impl_->put(k, makeScalar(Tag::Double, v)); }
void Archive::set(const std::string& k, bool v) { uint8_t b = v ? 1 : 0; impl_->put(k, makeScalar(Tag::Bool, b)); }
void Archive::set(const std::string& k, const std::string& v) { impl_->put(k, makeString(v)); }

void Archive::set(const std::string& k, const glm::vec2& v) {
    auto n = std::make_shared<Node>(); n->tag = Tag::Vec2;
    n->scalar.resize(8);  std::memcpy(n->scalar.data(), &v.x, 8);  impl_->put(k, n);
}
void Archive::set(const std::string& k, const glm::vec3& v) {
    auto n = std::make_shared<Node>(); n->tag = Tag::Vec3;
    n->scalar.resize(12); std::memcpy(n->scalar.data(), &v.x, 12); impl_->put(k, n);
}
void Archive::set(const std::string& k, const glm::vec4& v) {
    auto n = std::make_shared<Node>(); n->tag = Tag::Vec4;
    n->scalar.resize(16); std::memcpy(n->scalar.data(), &v.x, 16); impl_->put(k, n);
}
void Archive::set(const std::string& k, const glm::quat& v) {
    // store x,y,z,w
    float xyzw[4] = { v.x, v.y, v.z, v.w };
    auto n = std::make_shared<Node>(); n->tag = Tag::Quat;
    n->scalar.resize(16); std::memcpy(n->scalar.data(), xyzw, 16); impl_->put(k, n);
}
void Archive::set(const std::string& k, const glm::mat3& v) {
    // glm is column-major; value_ptr gives 9 contiguous floats col by col
    auto n = std::make_shared<Node>(); n->tag = Tag::Mat3;
    n->scalar.resize(36); std::memcpy(n->scalar.data(), &v[0][0], 36); impl_->put(k, n);
}
void Archive::set(const std::string& k, const glm::mat4& v) {
    auto n = std::make_shared<Node>(); n->tag = Tag::Mat4;
    n->scalar.resize(64); std::memcpy(n->scalar.data(), &v[0][0], 64); impl_->put(k, n);
}

// ---------------------------------------------------------------------------
// write nested
// ---------------------------------------------------------------------------
void Archive::set(const std::string& k, Archive&& child) {
    // child's root is an Object node; adopt it directly
    impl_->put(k, child.impl_->root);
}

void Archive::append(const std::string& k, Archive&& child) {
    auto it = impl_->root->fields.find(k);
    std::shared_ptr<Node> arr;
    if (it == impl_->root->fields.end() || it->second->tag != Tag::Array) {
        arr = std::make_shared<Node>();
        arr->tag = Tag::Array;
        impl_->put(k, arr);
    }
    else {
        arr = it->second;
    }
    arr->elements.push_back(child.impl_->root);
}

// ---------------------------------------------------------------------------
// read scalars
// ---------------------------------------------------------------------------
bool Archive::get(const std::string& k, int& out) const { return readScalar(impl_->find(k), Tag::Int, out); }
bool Archive::get(const std::string& k, unsigned& out) const { return readScalar(impl_->find(k), Tag::UInt, out); }
bool Archive::get(const std::string& k, float& out) const { return readScalar(impl_->find(k), Tag::Float, out); }
bool Archive::get(const std::string& k, double& out) const { return readScalar(impl_->find(k), Tag::Double, out); }

bool Archive::get(const std::string& k, bool& out) const {
    uint8_t b;
    if (!readScalar(impl_->find(k), Tag::Bool, b)) return false;
    out = (b != 0); return true;
}

bool Archive::get(const std::string& k, std::string& out) const {
    const Node* n = impl_->find(k);
    if (!n || n->tag != Tag::String) return false;
    out.assign(n->scalar.begin(), n->scalar.end());
    return true;
}

bool Archive::get(const std::string& k, glm::vec2& out) const {
    const Node* n = impl_->find(k);
    if (!n || n->tag != Tag::Vec2 || n->scalar.size() != 8) return false;
    std::memcpy(&out.x, n->scalar.data(), 8); return true;
}
bool Archive::get(const std::string& k, glm::vec3& out) const {
    const Node* n = impl_->find(k);
    if (!n || n->tag != Tag::Vec3 || n->scalar.size() != 12) return false;
    std::memcpy(&out.x, n->scalar.data(), 12); return true;
}
bool Archive::get(const std::string& k, glm::vec4& out) const {
    const Node* n = impl_->find(k);
    if (!n || n->tag != Tag::Vec4 || n->scalar.size() != 16) return false;
    std::memcpy(&out.x, n->scalar.data(), 16); return true;
}
bool Archive::get(const std::string& k, glm::quat& out) const {
    const Node* n = impl_->find(k);
    if (!n || n->tag != Tag::Quat || n->scalar.size() != 16) return false;
    float xyzw[4];
    std::memcpy(xyzw, n->scalar.data(), 16);
    out.x = xyzw[0]; out.y = xyzw[1]; out.z = xyzw[2]; out.w = xyzw[3];
    return true;
}
bool Archive::get(const std::string& k, glm::mat3& out) const {
    const Node* n = impl_->find(k);
    if (!n || n->tag != Tag::Mat3 || n->scalar.size() != 36) return false;
    std::memcpy(&out[0][0], n->scalar.data(), 36); return true;
}
bool Archive::get(const std::string& k, glm::mat4& out) const {
    const Node* n = impl_->find(k);
    if (!n || n->tag != Tag::Mat4 || n->scalar.size() != 64) return false;
    std::memcpy(&out[0][0], n->scalar.data(), 64); return true;
}

// ---------------------------------------------------------------------------
// read nested
// ---------------------------------------------------------------------------
bool Archive::has(const std::string& k) const { return impl_->find(k) != nullptr; }

Archive Archive::get(const std::string& k) const {
    Impl* childImpl = new Impl();
    const Node* n = impl_->find(k);
    if (n && n->tag == Tag::Object)
        childImpl->root = std::make_shared<Node>(*n);   // deep-ish copy (shared children)
    return Archive(childImpl);
}

size_t Archive::size(const std::string& k) const {
    const Node* n = impl_->find(k);
    if (!n || n->tag != Tag::Array) return 0;
    return n->elements.size();
}

Archive Archive::at(const std::string& k, size_t index) const {
    Impl* childImpl = new Impl();
    const Node* n = impl_->find(k);
    if (n && n->tag == Tag::Array && index < n->elements.size()) {
        const auto& elem = n->elements[index];
        if (elem->tag == Tag::Object)
            childImpl->root = std::make_shared<Node>(*elem);
    }
    return Archive(childImpl);
}