#pragma once

#include <scene/components/BehaviourComponent.h>
#include <scene/components/ComponentFactory.h>
#include <property/SerializeMacro.h>

class Archive;

#define SCRIPT(ScriptName) \
    class ScriptName : public BehaviourComponent {

// For registering ONLY user level scripts/components
// lambda is declared inline in a static context, executes before main()
#define END_SCRIPT(ScriptName) \
    }; \
    inline bool _autoReg_##ScriptName = []() { \
        ComponentTypeInfo<ScriptName>::name = #ScriptName; \
        unsigned int UID = componentTypeUID<ScriptName>(); \
        __componentTypeUIDToString(UID, true, #ScriptName); \
        ComponentFactory::registerScript(#ScriptName, [](Entity& e, const Archive& arch) { \
            auto& c = e.addComponent<ScriptName>(); \
            c.deserialize(arch); \
        }); \
        return true; \
    }()