#include "SFSE/Stub.h"

#include "sfse_common/Relocation.h"
#include "sfse_common/SafeWrite.h"

DLLEXPORT constinit auto SFSEPlugin_Version = []() noexcept {
	SFSE::PluginVersionData data{};

	data.PluginVersion(Plugin::Version);
	data.PluginName(Plugin::NAME);
	data.AuthorName(Plugin::AUTHOR);
	data.UsesSigScanning(true);
	//data.UsesAddressLibrary(true);
	data.HasNoStructUse(true);
	//data.IsLayoutDependent(true);
	data.CompatibleVersions({ RUNTIME_VERSION_1_7_23 });

	return data;
}();

template <std::uintptr_t ADDR, std::ptrdiff_t OFF>
class hkCheckModsLoaded
{
public:
	static void Install()
	{
		const RelocAddr<std::uintptr_t> target(ADDR + OFF);
		DKUtil::Hook::write_call<5>(target.getUIntPtr(), CheckModsLoaded);
	}

private:
	static bool CheckModsLoaded(void*, bool)
	{
		return false;
	}
};

template <std::uintptr_t ADDR, std::ptrdiff_t OFF>
class hkShowUsingConsoleMayDisableAchievements
{
public:
	static void Install()
	{
		const RelocAddr<std::uintptr_t> target(ADDR + OFF);
		DKUtil::Hook::write_call<5>(target.getUIntPtr(), ShowUsingConsoleMayDisableAchievements);
	}

private:
	static void ShowUsingConsoleMayDisableAchievements(void*)
	{
		return;
	}
};

namespace
{
	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept
	{
		switch (a_msg->type) {
		case SFSE::MessagingInterface::kPostLoad:
			{
				// Disable check
				hkCheckModsLoaded<0x01495C14, 0x02ED>::Install();
				hkCheckModsLoaded<0x01FDCA50, 0x13FE>::Install();
				hkCheckModsLoaded<0x02338BD8, 0x005B>::Install();
				hkCheckModsLoaded<0x023AC9AC, 0x03AC>::Install();
				hkCheckModsLoaded<0x023B30F8, 0x002F>::Install();
				hkCheckModsLoaded<0x02579C34, 0x1471>::Install();
				hkCheckModsLoaded<0x0258846C, 0x1075>::Install();
				hkCheckModsLoaded<0x029FC380, 0x007B>::Install();

				// Disable stupid message
				hkShowUsingConsoleMayDisableAchievements<0x02879B60, 0x67>::Install();
				break;
			}
		default:
			break;
		}
	}
}

/**
// for preload plugins
void SFSEPlugin_Preload(SFSE::LoadInterface* a_sfse);
/**/

DLLEXPORT bool SFSEAPI SFSEPlugin_Load(SFSEInterface* a_sfse)
{
#ifndef NDEBUG
	while (!IsDebuggerPresent()) {
		Sleep(100);
	}
#endif

	SFSE::Init(a_sfse);

	DKUtil::Logger::Init(Plugin::NAME, std::to_string(Plugin::Version));

	INFO("{} v{} loaded", Plugin::NAME, Plugin::Version);

	// do stuff
	SFSE::AllocTrampoline(1 << 10);

	SFSE::GetMessagingInterface()->RegisterListener(MessageCallback);

	return true;
}
