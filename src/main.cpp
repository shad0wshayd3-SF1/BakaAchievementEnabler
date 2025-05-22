namespace Hooks
{
	namespace hkShowUsingConsoleMayDisableAchievements
	{
		static void Install()
		{
			static REL::Relocation target{ REL::ID(113625) };
			static constexpr auto  TARGET_ADDR{ 0xE9 };
			static constexpr auto  TARGET_RETN{ 0xEE };
			static constexpr auto  TARGET_FILL{ TARGET_RETN - TARGET_ADDR };
			target.write_fill<TARGET_ADDR>(REL::NOP, TARGET_FILL);
		}
	}

	namespace hkAddAchievement
	{
		static void Install()
		{
			static REL::Relocation target{ REL::ID(117359) };
			static constexpr auto  TARGET_ADDR{ 0x082 };
			static constexpr auto  TARGET_RETN{ 0x104 };
			static constexpr auto  TARGET_FILL{ TARGET_RETN - TARGET_ADDR };
			target.write_fill<TARGET_ADDR>(REL::NOP, TARGET_FILL);
		}
	}

	namespace hkNoModdedTag
	{
		static void Install()
		{
			static REL::Relocation target{ REL::ID(114443) };
			static constexpr auto  TARGET_ADDR{ 0xAE };
			static constexpr auto  TARGET_RETN{ 0xD4 };
			static constexpr auto  TARGET_FILL{ TARGET_RETN - TARGET_ADDR };
			target.write_fill<TARGET_ADDR>(REL::NOP, TARGET_FILL);
		}
	}

	static void Install()
	{
		// Disable "$UsingConsoleMayDisableAchievements" message
		hkShowUsingConsoleMayDisableAchievements::Install();

		// Disable AddAchievement checks
		hkAddAchievement::Install();

		// Disable modded tag
		hkNoModdedTag::Install();
	}
}

namespace
{
	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept
	{
		switch (a_msg->type)
		{
		case SFSE::MessagingInterface::kPostLoad:
			Hooks::Install();
			break;
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
