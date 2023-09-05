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

namespace REL
{
	std::uintptr_t write_vfunc(std::uintptr_t a_addr, std::size_t a_idx, std::uintptr_t a_newFunc)
	{
		const auto addr = a_addr + (sizeof(void*) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t*>(addr);
		safeWrite64(addr, a_newFunc);
		return result;
	}
}

template <std::uintptr_t ADDR, std::ptrdiff_t OFF>
class hkCheckModsLoaded
{
public:
	static void Install()
	{
		static RelocAddr<std::uintptr_t> target(ADDR + OFF);
		DKUtil::Hook::write_call<5>(target.getUIntPtr(), CheckModsLoaded);
	}

private:
	static bool CheckModsLoaded(void*, bool)
	{
		return false;
	}
};

template <std::uintptr_t ADDR, std::ptrdiff_t OFF>
class hkShowLoadVanillaSaveWithMods
{
public:
	static void Install()
	{
		static RelocAddr<std::uintptr_t> target(ADDR + OFF);
		DKUtil::Hook::write_call<5>(target.getUIntPtr(), ShowLoadVanillaSaveWithMods);
	}

private:
	static void ShowLoadVanillaSaveWithMods()
	{
		static RelocPtr<std::uint32_t> dword(0x059055E4);
		(*dword.getPtr()) &= ~2;
		static RelocAddr<void (*)(void*, void*, std::int32_t, std::int32_t, void*)> func(0x023A9F24);
		return func(nullptr, nullptr, 0, 0, nullptr);
	}
};

template <std::uintptr_t ADDR, std::ptrdiff_t OFF>
class hkShowUsingConsoleMayDisableAchievements
{
public:
	static void Install()
	{
		static RelocAddr<std::uintptr_t> target(ADDR + OFF);
		DKUtil::Hook::write_call<5>(target.getUIntPtr(), ShowUsingConsoleMayDisableAchievements);
	}

private:
	static void ShowUsingConsoleMayDisableAchievements(void*)
	{
		return;
	}
};

class hkPlayerCharacterSaveGame
{
public:
	static void Install()
	{
		static RelocAddr<std::uintptr_t> target(0x044DB6F0);
		auto orig = REL::write_vfunc(target.getUIntPtr(), 0x1A, reinterpret_cast<std::uintptr_t>(PlayerCharacterSaveGame));
		_PlayerCharacterSaveGame = reinterpret_cast<func_t>(orig);
	}

private:
	static void PlayerCharacterSaveGame(void* a_this, void* a_buffer)
	{
		static RelocPtr<bool> hasModded(0x05905958);
		(*hasModded.getPtr()) = false;

		static RelocPtr<std::byte*> playerCharacter(0x05594D28);
		auto flag = reinterpret_cast<bool*>((*playerCharacter.getPtr()) + 0x10E6);
		*flag &= ~4;

		return _PlayerCharacterSaveGame(a_this, a_buffer);
	}

	using func_t = std::add_pointer_t<void(void*, void*)>;
	inline static func_t _PlayerCharacterSaveGame;
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

				// Disable "$LoadVanillaSaveWithMods" message
				hkShowLoadVanillaSaveWithMods<0x023A9F24, 0x9F>::Install();

				// Disable "$UsingConsoleMayDisableAchievements" message
				hkShowUsingConsoleMayDisableAchievements<0x02879B60, 0x67>::Install();

				// Disable modded flag when saving
				hkPlayerCharacterSaveGame::Install();
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

	SFSE::AllocTrampoline(1 << 10);

	SFSE::GetMessagingInterface()->RegisterListener(MessageCallback);

	return true;
}
