class Hooks
{
public:
	static void Install()
	{
		// Disable "$UsingConsoleMayDisableAchievements" message
		hkShowUsingConsoleMayDisableAchievements::Install();

		// Disable ConsoleCommand check in CheckModsLoaded
		hkCheckModsLoaded::Install();

		// Disable UserContent func1
		//hkUserContentFunc1<1869655, 0x066>::Install();
		//hkUserContentFunc1<1869657, 0x086>::Install();
		//hkUserContentFunc1<1869669, 0x936>::Install();

		// Disable UserContent func2 "Creations" "data" "%s%s"
		//hkUserContentFunc2<1869955, 0x13B>::Install();
		//hkUserContentFunc2<1869962, 0x15A>::Install();
		//hkUserContentFunc2<1870073, 0x053>::Install();
		hkUserContentFunc2<1870077, 0x026>::Install();
		//hkUserContentFunc2<1870083, 0x05A>::Install();
		//hkUserContentFunc2<1870084, 0x6B1>::Install();
		hkUserContentFunc2<1870086, 0xE6C>::Install();
		//hkUserContentFunc2<1870098, 0xA1B>::Install();
	}

private:
	class hkShowUsingConsoleMayDisableAchievements
	{
	public:
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::ID(166267), 0x67 };
			REL::safe_fill(target.address(), REL::NOP, 0x05);
		}
	};

	class hkCheckModsLoaded
	{
	public:
		static void Install()
		{
			struct PatchConsoleBoolean : Xbyak::CodeGenerator
			{
				PatchConsoleBoolean(std::uintptr_t a_retn)
				{
					mov(rdi, a_retn);
					jmp(rdi);
				}
			};

			static REL::Relocation<std::uintptr_t> target{ REL::ID(85097) };
			static constexpr auto TARGET_ADDR{ 0x28 };
			static constexpr auto TARGET_RETN{ 0x38 };
			static constexpr auto TARGET_FILL{ 0x09 };
			REL::safe_fill(target.address() + TARGET_ADDR, REL::NOP, TARGET_FILL);

			auto code = PatchConsoleBoolean(
				target.address() + TARGET_RETN);
			auto& trampoline = SFSE::GetTrampoline();
			trampoline.write_branch<5>(target.address() + TARGET_ADDR, trampoline.allocate(code));
		}
	};

	class detail
	{
	public:
		static void UserContentPatch(void* a_this)
		{
			auto idx = RE::stl::adjust_pointer<std::uint8_t>(a_this, 0x54);
			*idx = 1;
		}
	};

	template <std::uintptr_t ID, std::ptrdiff_t OFF>
	class hkUserContentFunc1
	{
	public:
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::ID(ID), OFF };
			auto& trampoline = SFSE::GetTrampoline();
			_UserContentFunc1 = trampoline.write_call<5>(target.address(), UserContentFunc1);
		}

	private:
		static void UserContentFunc1(void* a_this, void* a_arg2)
		{
			_UserContentFunc1(a_this, a_arg2);
			detail::UserContentPatch(a_this);
		}

		inline static REL::Relocation<decltype(&UserContentFunc1)> _UserContentFunc1;
	};

	template <std::uintptr_t ID, std::ptrdiff_t OFF>
	class hkUserContentFunc2
	{
	public:
		static void Install()
		{
			static REL::Relocation<std::uintptr_t> target{ REL::ID(ID), OFF };
			auto& trampoline = SFSE::GetTrampoline();
			_UserContentFunc2 = trampoline.write_call<5>(target.address(), UserContentFunc2);
		}

	private:
		static void UserContentFunc2(void* a_this)
		{
			_UserContentFunc2(a_this);
			detail::UserContentPatch(a_this);
		}

		inline static REL::Relocation<decltype(&UserContentFunc2)> _UserContentFunc2;
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

	SFSE::AllocTrampoline(64);
	SFSE::GetMessagingInterface()->RegisterListener(MessageCallback);

	return true;
}
