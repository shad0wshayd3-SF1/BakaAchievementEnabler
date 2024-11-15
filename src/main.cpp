namespace Hooks
{
	namespace hkShowUsingConsoleMayDisableAchievements
	{
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::ID(166267) };
			static constexpr auto TARGET_ADDR{ 0x67 };
			static constexpr auto TARGET_RETN{ 0x6C };
			static constexpr auto TARGET_FILL{ TARGET_RETN - TARGET_ADDR };
			REL::safe_fill(target.address() + TARGET_ADDR, REL::NOP, TARGET_FILL);
		}
	}

	namespace hkAddAchievement
	{
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::ID(171028) };
			static constexpr auto TARGET_ADDR{ 0x71 };
			static constexpr auto TARGET_RETN{ 0xF3 };
			static constexpr auto TARGET_FILL{ TARGET_RETN - TARGET_ADDR };
			REL::safe_fill(target.address() + TARGET_ADDR, REL::NOP, TARGET_FILL);
		}
	}

	static void Install()
	{
		// Disable "$UsingConsoleMayDisableAchievements" message
		hkShowUsingConsoleMayDisableAchievements::Install();

		// Disable AddAchievement checks
		hkAddAchievement::Install();
	}
}

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
	SFSE::GetMessagingInterface()->RegisterListener(MessageCallback);

	return true;
}
