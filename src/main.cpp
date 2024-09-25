class Hooks
{
public:
	static void Install()
	{
		// Disable "$LoadVanillaSaveWithMods" message
		hkShowLoadVanillaSaveWithMods::Install();

		// Disable "$UsingConsoleMayDisableAchievements" message
		hkShowUsingConsoleMayDisableAchievements::Install();

		// Disable new AddAchievement check
		hkAddAchievement::Install();

		// Disable ConfirmNewWithModsCallback
		hkConfirmNewWithModsCallback::Install();
	}

private:
	class hkShowLoadVanillaSaveWithMods
	{
	public:
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::ID(1869664), 0x10B };
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

	class hkShowUsingConsoleMayDisableAchievements
	{
	public:
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::ID(166267), 0x67 };
			REL::safe_fill(target.address(), REL::NOP, 0x05);
		}
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

	SFSE::AllocTrampoline(16);
	SFSE::GetMessagingInterface()->RegisterListener(MessageCallback);

	return true;
}
