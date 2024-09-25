class Hooks
{
public:
	static void Install()
	{
		// Disable "$LoadVanillaSaveWithMods" message
		hkShowLoadVanillaSaveWithMods::Install();

		// Disable "$UsingConsoleMayDisableAchievements" message
		hkShowUsingConsoleMayDisableAchievements::Install();

		// Disable ConsoleCommand check in CheckModsLoaded
		hkCheckModsLoaded::Install();

		// Disable "ConfirmNewWithModsCallback" message
		hkConfirmNewWithModsCallback::Install();

		// Disable UserContent func1
		//hkUserContentFunc1<1869655, 0x066>::Install();
		//hkUserContentFunc1<1869657, 0x086>::Install();
		//hkUserContentFunc1<1869669, 0x936>::Install();

		// Disable UserContent func2
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

	class hkCheckModsLoaded
	{
	public:
		static void Install()
		{
			struct PatchConsoleBoolean :
				Xbyak::CodeGenerator
			{
				PatchConsoleBoolean(std::uintptr_t a_end)
				{
					mov(rdi, a_end);
					jmp(rdi);
				}
			};

			static REL::Relocation<std::uintptr_t> target{ REL::ID(85097), 0x28 };
			REL::safe_fill(target.address(), REL::NOP, 0x09);

			auto& trampoline = SFSE::GetTrampoline();
			auto patch = PatchConsoleBoolean(target.address() + 0x10);
			auto alloc = trampoline.allocate(patch);
			trampoline.write_branch<5>(target.address(), reinterpret_cast<std::uintptr_t>(alloc));
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

	SFSE::AllocTrampoline(128);
	SFSE::GetMessagingInterface()->RegisterListener(MessageCallback);

	return true;
}
