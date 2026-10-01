#pragma once

#include <scene/components/BehaviourComponent.h>
#include <scene/components/ComponentFactory.h>
#include <property/SerializeMacro.h>

#include <optional>

class Archive;

#define SCRIPT(ScriptName) \
    class ScriptName : public BehaviourComponent {

// for registering ONLY user level scripts/components
// lambda is declared inline in a static context, executes before main()
// compiled into a DLL, executes at library load-time
/*
*/
#define END_SCRIPT(ScriptName) \
    }; \
    inline bool _autoReg_##ScriptName = []() { \
        ComponentTypeInfo<ScriptName>::name = #ScriptName; \
        unsigned int UID = componentTypeUID<ScriptName>(); \
        __componentTypeUIDToString(UID, true, #ScriptName); \
        ComponentFactory::registerScript(#ScriptName, [](Entity& e, const Archive& arch) { \
            auto& c = e.addComponent<ScriptName>(); \
            c.deserialize(arch); \
        }, COMPONENT_OPS(ScriptName)); \
        return true; \
    }()
