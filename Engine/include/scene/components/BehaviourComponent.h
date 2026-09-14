#pragma once

#include <scene/components/Component.h>
#include <property/Property.h>
#include <property/SerializeMacro.h>

class Scene;
class Archive;

// Component already derives from PropertyHolder; re-inheriting it here would give
// BehaviourComponent two PropertyHolder subobjects (ambiguous getProperties, C4584).
class BehaviourComponent : public Component {
public:
    friend class Scene;

    virtual ~BehaviourComponent() = default;
    virtual void onStart() {}
    virtual void onUpdate(float dt) {}

    void serialize(Archive& arch) const override;
    void deserialize(const Archive& arch) override;

private:
    void tick(float dt) {
        if (!started) {
            onStart();
            started = true;
        }
        onUpdate(dt);
    }

    bool started = false;
};