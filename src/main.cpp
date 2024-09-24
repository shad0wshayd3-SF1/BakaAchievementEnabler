class Hooks
{
public:
	static void Install()
	{
		// Disable CheckModsLoaded
		hkCheckModsLoaded<131234, 0x13FE>::Install();  // Main_CreatePlayerCharacter
		hkCheckModsLoaded<147862, 0x03AC>::Install();  // BGSSaveLoadManager::GenerateSaveFileName
		hkCheckModsLoaded<153566, 0x1410>::Install();  // PlayerCharacter::LoadGame
		hkCheckModsLoaded<153715, 0x1097>::Install();  // PlayerCharacter::SaveGame

		// Disable "$LoadVanillaSaveWithMods" message
		hkShowLoadVanillaSaveWithMods<1869664, 0x10B>::Install();

		// Disable "$UsingConsoleMayDisableAchievements" message
		hkShowUsingConsoleMayDisableAchievements<166267, 0x67>::Install();

		// Disable modded flag when saving
		hkPlayerCharacterSaveGame::Install();

		// Disable new AddAchievement check
		hkAddAchievement::Install();

		// Disable ConfirmNewWithModsCallback
		hkConfirmNewWithModsCallback::Install();
	}

private:
	template <std::uintptr_t ID, std::ptrdiff_t OFF>
	class hkCheckModsLoaded
	{
	public:
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::ID(ID), OFF };
			auto& trampoline = SFSE::GetTrampoline();
			trampoline.write_call<5>(target.address(), CheckModsLoaded);
		}

	private:
		static bool CheckModsLoaded(void*, bool)
		{
			return false;
		}
	};

	template <std::uintptr_t ID, std::ptrdiff_t OFF>
	class hkShowLoadVanillaSaveWithMods
	{
	public:
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::ID(ID), OFF };
			auto& trampoline = SFSE::GetTrampoline();
			trampoline.write_call<5>(target.address(), ShowLoadVanillaSaveWithMods);
		}

	private:
		static void ShowLoadVanillaSaveWithMods()
		{
			static REL::Relocation<std::uint32_t*> dword{ REL::ID(1969023) };
			(*dword.get()) &= ~2;

			static REL::Relocation<void (*)(void*, void*, std::int32_t, std::int32_t, void*)> func{ REL::ID(1869664) };
			return func(nullptr, nullptr, 0, 0, nullptr);
		}
	};

	template <std::uintptr_t ID, std::ptrdiff_t OFF>
	class hkShowUsingConsoleMayDisableAchievements
	{
	public:
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::ID(ID), OFF };
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
			static REL::Relocation<std::uintptr_t> target{ REL::ID(423292) };
			_PlayerCharacterSaveGame = target.write_vfunc(0x1A, PlayerCharacterSaveGame);
		}

	private:
		static void PlayerCharacterSaveGame(void* a_this, void* a_buffer)
		{
			static REL::Relocation<bool*> hasModded{ REL::ID(881136) };
			(*hasModded.get()) = false;

			static REL::Relocation<void**> PlayerCharacter{ REL::ID(865059) };
			auto flag = RE::stl::adjust_pointer<std::uint8_t>(*PlayerCharacter.get(), 0x1116);
			(*flag) &= ~4;

			return _PlayerCharacterSaveGame(a_this, a_buffer);
		}

		inline static REL::Relocation<decltype(&PlayerCharacterSaveGame)> _PlayerCharacterSaveGame;
	};

	class hkAddAchievement
	{
	public:
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::ID(171028), 0x72 };
			REL::safe_fill(target.address(), REL::NOP, 0x0D);
		}
	};

	class hkConfirmNewWithModsCallback
	{
	public:
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::ID(1869550), 0xCB };
			REL::safe_fill(target.address(), 0x02, 0x01);
		}
	};
};

namespace
{
	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept
	{
		switch (a_msg->type)
		{
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

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse)
{
	SFSE::Init(a_sfse);

	const auto plugin = SFSE::PluginVersionData::GetSingleton();
	SFSE::log::info("{} {} loaded", plugin->GetPluginName(), plugin->GetPluginVersion());

	SFSE::AllocTrampoline(1 << 8);
	SFSE::GetMessagingInterface()->RegisterListener(MessageCallback);

	return true;
}
