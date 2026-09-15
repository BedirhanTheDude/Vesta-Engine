#pragma once

#include <property/PropertyUtil.h>

#include <string>
#include <vector>
#include <map>
#include <unordered_set>

enum class PayloadType { Mesh, Texture }; // will add new payload types later

struct Payload {
    std::string filename;
    PayloadType type;
    void* ptr;
};

struct PayloadHash {
    size_t operator()(const Payload& p) const {
        return std::hash<std::string>()(p.filename) ^
            (std::hash<int>()(static_cast<int>(p.type)) << 1);
    }
};

struct PayloadEqual {
    bool operator()(const Payload& a, const Payload& b) const {
        return a.filename == b.filename && a.type == b.type;
    }
};

struct PayloadGroup {
    std::string name;
    std::map<unsigned int, Payload> payloads;
    std::unordered_set<Payload, PayloadHash, PayloadEqual> seen;

    unsigned int nextPayloadIdx = 0;

    void reset() {
        payloads.clear();
        seen.clear();
        nextPayloadIdx = 0;
    }
};

class PayloadHolder {
public:
    void registerPayload(const std::string& name, PayloadType type, void* ptr,
        unsigned int groupID = 0, std::string groupName = "defaultGroup") {
        if (groupID >= payloadGroups.size())
            payloadGroups.resize(groupID + 1);

        PayloadGroup& group = payloadGroups[groupID];
        if (group.name.empty())
            group.name = camelToWords(groupName);

        std::string propName = camelToWords(name);
        Payload prop = { propName, type, ptr };

        if (group.seen.count(prop)) return;

        group.payloads[group.nextPayloadIdx++] = prop;
        group.seen.insert(prop);
    }

    std::map<unsigned int, Payload>& getPayloads(unsigned int groupID = 0) {
        static std::map<unsigned int, Payload> empty;
        if (groupID < payloadGroups.size()) return payloadGroups[groupID].payloads;
        return empty;
    }
    std::vector<PayloadGroup>& getPayloadGroups() { return payloadGroups; }

protected:
    std::vector<PayloadGroup> payloadGroups;
};