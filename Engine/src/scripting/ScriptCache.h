#pragma once

class Scene;

class ScriptCache {
public:
	static void saveScriptState(const Scene& scene);
	static void loadScriptState(Scene& scene);
};