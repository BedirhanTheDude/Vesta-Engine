#pragma once

#include <string>

struct HINSTANCE__;
using HMODULE = HINSTANCE__*;

class Scene; // remove coupling!

class ScriptCompiler {
public:
	static HMODULE getCurrentDLLHandle();
	static HMODULE reloadScripts(Scene& scene);
	static void poll(float dt, Scene& scene);
	static void setPollingEnabled(bool enabled) { pollingEnabled = enabled; }

	static bool compile();
	static HMODULE loadDLL();
	static void unloadDLL(HMODULE handle);
private:
	static HMODULE currentDLLHandle;
	static bool pollingEnabled;
};