class Hooks
{
public:
	static void Install()
	{
		// Disable check
		hkCheckModsLoaded<0x01495574, 0x02ED>::Install();
		hkCheckModsLoaded<0x01FDC560, 0x13FE>::Install();
		hkCheckModsLoaded<0x0233BCF8, 0x005B>::Install();
		hkCheckModsLoaded<0x023AFADC, 0x03AC>::Install();
		hkCheckModsLoaded<0x023B6228, 0x002F>::Install();
		hkCheckModsLoaded<0x0257CBD4, 0x1471>::Install();
		hkCheckModsLoaded<0x0258B40C, 0x1075>::Install();
		hkCheckModsLoaded<0x029FFCA0, 0x007B>::Install();

		// Disable "$LoadVanillaSaveWithMods" message
		hkShowLoadVanillaSaveWithMods<0x023AD054, 0x9F>::Install();

		// Disable "$UsingConsoleMayDisableAchievements" message
		hkShowUsingConsoleMayDisableAchievements<0x0287CAF0, 0x67>::Install();

		// Disable modded flag when saving
		hkPlayerCharacterSaveGame::Install();
	}

private:
	template <std::uintptr_t ADDR, std::ptrdiff_t OFF>
	class hkCheckModsLoaded
	{
	public:
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::Offset(ADDR), OFF };
			auto& trampoline = SFSE::GetTrampoline();
			trampoline.write_call<5>(target.address(), CheckModsLoaded);
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
			static REL::Relocation<std::uintptr_t> target{ REL::Offset(ADDR), OFF };
			auto& trampoline = SFSE::GetTrampoline();
			trampoline.write_call<5>(target.address(), ShowLoadVanillaSaveWithMods);
		}

	private:
		static void ShowLoadVanillaSaveWithMods()
		{
			static REL::Relocation<std::uint32_t*> dword{ REL::Offset(0x059095E4) };
			(*dword.get()) &= ~2;

			static REL::Relocation<void (*)(void*, void*, std::int32_t, std::int32_t, void*)> func{ REL::Offset(0x023AD054) };
			return func(nullptr, nullptr, 0, 0, nullptr);
		}
	};

	template <std::uintptr_t ADDR, std::ptrdiff_t OFF>
	class hkShowUsingConsoleMayDisableAchievements
	{
	public:
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::Offset(ADDR), OFF };
			auto& trampoline = SFSE::GetTrampoline();
			trampoline.write_call<5>(target.address(), ShowUsingConsoleMayDisableAchievements);
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
			static REL::Relocation<std::uintptr_t> target{ REL::Offset(0x044DF698) };
			_PlayerCharacterSaveGame = target.write_vfunc(0x1A, PlayerCharacterSaveGame);
		}

	private:
		static void PlayerCharacterSaveGame(void* a_this, void* a_buffer)
		{
			static REL::Relocation<bool*> hasModded{ REL::Offset(0x0590997A) };
			(*hasModded.get()) = false;

			static REL::Relocation<void**> PlayerCharacter{ REL::Offset(0x05598CB8) };
			auto flag = RE::stl::adjust_pointer<bool>(*PlayerCharacter.get(), 0x10E6);
			(*flag) &= ~4;

			return _PlayerCharacterSaveGame(a_this, a_buffer);
		}

		inline static REL::Relocation<decltype(&PlayerCharacterSaveGame)> _PlayerCharacterSaveGame;
	};
};

DLLEXPORT constinit auto SFSEPlugin_Version = []() noexcept {
	SFSE::PluginVersionData data{};

	data.PluginVersion(Plugin::Version);
	data.PluginName(Plugin::NAME);
	data.AuthorName(Plugin::AUTHOR);
	data.UsesSigScanning(false);
	data.UsesAddressLibrary(false);
	data.HasNoStructUse(true);
	data.IsLayoutDependent(false);
	data.CompatibleVersions({ SFSE::RUNTIME_LATEST });

	return data;
}();

namespace
{
	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept
	{
		switch (a_msg->type) {
		case SFSE::MessagingInterface::kPostLoad:
			{
				Hooks::Install();
				break;
			}
		default:
			break;
		}
	}
}

DLLEXPORT bool SFSEAPI SFSEPlugin_Load(const SFSE::LoadInterface* a_sfse)
{
#ifndef NDEBUG
	while (!IsDebuggerPresent()) {
		Sleep(100);
	}
#endif

	SFSE::Init(a_sfse);

	DKUtil::Logger::Init(Plugin::NAME, std::to_string(Plugin::Version));

	INFO("{} v{} loaded", Plugin::NAME, Plugin::Version);

	SFSE::AllocTrampoline(1 << 8);

	SFSE::GetMessagingInterface()->RegisterListener(MessageCallback);

	return true;
}
