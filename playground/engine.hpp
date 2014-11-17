#pragma once

#include "arc/string/StringView.hpp"
#include "arc/lua/State.hpp"
#include "arc/logging/log.hpp"
#include "arc/hash/StringHash.hpp"

namespace arc
{
	namespace engine
	{
		struct Config
		{
			uint8 gl_version_major = 4;
			uint8 gl_version_minor = 4;

			uint16 window_width = 1280;
			uint16 window_height = 720;

			bool window_hidden = false;
			bool fullscreen = false;
		};

		void initialize(const Config& config);

		void shutdown();

		void deprecated_swap();
		
		void deprecated_update();

		memory::Allocator& longterm_allocator();

		// Updates /////////////////////////////////////////////

		bool register_frame_begin_cb(StringHash name, const std::function<void(double)>& cb);
		bool unregister_frame_begin_cb(StringHash name);

		// Subsystems //////////////////////////////////////////

		class Subsystem
		{
		public:
			virtual const char* name() = 0;
		protected:
			virtual bool initialize(lua::State& config) = 0;
			virtual bool finalize() = 0;
		public:
			bool assert_initialized(lua::State& config);
			bool assert_finalized();
		private:
			bool m_initialized = false;
			bool m_finalized = false;
		};

		template <typename T>
		T* add_subsystem();

		bool initialize_subsystems(lua::State& config);
		bool finalize_subsystems();

		template <typename T>
		T* get_subsystem();

		// Helper //////////////////////////////////////////////

		#define ARC_SUBSYSTEM_DECLARATION(SystemName)			\
			public:												\
				virtual const char* name();						\
				static uint64 SubsystemType();					\
			protected:											\
				virtual bool initialize(lua::State& config);	\
				virtual bool finalize();						\
				SystemName(memory::Allocator& alloc);			\
			private:											\
				friend class arc::memory::Allocator;			\
				ARC_NO_COPY(SystemName)							\

		#define ARC_SUBSYSTEM_DEFINITION(FullSystemName)										\
				const char* FullSystemName::name() { return #FullSystemName; }					\
				uint64 FullSystemName::SubsystemType() { return SH(#FullSystemName).value(); }	\

		// Internals ///////////////////////////////////////////

		Subsystem*& _assert_subsystem(uint64 type);
		Subsystem*  _get_subsystem(uint64 type);

		// Implementation //////////////////////////////////////

		template <typename T>
		T* add_subsystem()
		{
			uint64 type = T::SubsystemType();
			
			auto& sys = _assert_subsystem(type);
			if (sys != nullptr)
			{
				LOG_ERROR("A subsystem with of type ", type, " was already added: ", sys->name());
				ARC_ASSERT(false,"");
				return nullptr;
			}

			auto new_sys = engine::longterm_allocator().create<T>(engine::longterm_allocator());
			sys = new_sys;
			return new_sys;
		}

		template <typename T>
		T* get_subsystem()
		{
			uint64 type = T::SubsystemType();
			auto sys = _get_subsystem(type);
			ARC_ASSERT(sys != nullptr, "could not get subsystem");
			return static_cast<T*>(sys);
		}

	}
}