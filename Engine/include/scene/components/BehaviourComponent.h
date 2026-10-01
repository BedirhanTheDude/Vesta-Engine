#pragma once

#include <scene/components/Component.h>

#include <persistance/Serializable.h>
#include <property/Property.h>
#include <property/CallbackProperty.h>
#include <property/Payload.h>
#include <property/SerializeMacro.h>

class Scene;
class Archive;

// Base class for user-level scripts
class BehaviourComponent : public Component, public PropertyHolder,
    public CallbackPropertyHolder, public PayloadHolder {
public:
    friend class Scene;

    virtual ~BehaviourComponent() = default;

    virtual bool onAttach() { return true; }
    virtual void onDetach() {}

    virtual void onStart() {}
    virtual void onUpdate(float dt) {}
    virtual void onLateUpdate(float dt) {}

    void serialize(Archive& arch) const override;
    void deserialize(const Archive& arch) override;

    unsigned int getTypeUID() const { return typeUID; }

private:
    void tick(float dt) {
        if (!started) {
            onStart();
            started = true;
        }
        onUpdate(dt);
    }

    bool started = false;

    unsigned int typeUID = 0; // set by Scene when the behaviour is attached to an entity
};
