#pragma once

#include <stdint.h>
#include <stddef.h>
#include <sys/mman.h>

namespace Injector {
	typedef unsigned long       DWORD;

	

	class game_version_manager
	{
	public:
		const char* PluginName;
		bool Detect() { return true; }
	};

	/*
	 *  address_manager
	 *      Address translator from 1.0 executables to other executables offsets
	 *      Inherits from game_version_manager ;)
	 */
	class address_manager : public game_version_manager
	{
	private:
		address_manager()
		{
			this->Detect();
		}

		// You could implement your translator for the address your plugin uses
		// If not implemented, the translator won't translate anything, just return the samething as before
#ifdef INJECTOR_GVM_HAS_TRANSLATOR
		void* translator(void* p);
#else
		void* translator(void* p) { return p; }
#endif

	public:
		// Translates address p to the running executable pointer
		void* translate(void* p)
		{
			return translator(p);
		}


	public:
		// Address manager singleton
		static address_manager& singleton()
		{
			static address_manager m;
			return m;
		}

		// Static version of translate()
		static void* translate_address(void* p)
		{
			return singleton().translate(p);
		}

		//
		static void set_name(const char* modname)
		{
			singleton().PluginName = modname;
		}

	public:
		// Functors for memory translation:

		// Translates nothing translator
		struct fn_mem_translator_nop
		{
			void* operator()(void* p) const
			{
				return p;
			}
		};

		// Real translator
		struct fn_mem_translator
		{
			void* operator()(void* p) const
			{
				return translate_address(p);
			}
		};
	};


	/*
	 *  auto_pointer
	 *      Casts itself to another pointer type in the lhs
	 */
	union auto_pointer {
	protected:
		friend union memory_pointer_tr;
		template<class T> friend union basic_memory_pointer;

		void* p;
		uintptr_t a;

	public:
		auto_pointer() : p(0) {}

		auto_pointer(const auto_pointer& x) : p(x.p) {}

		explicit auto_pointer(void* x) : p(x) {}

		explicit auto_pointer(uint32_t x) : a(x) {}
		
		bool is_null() const {
			return this->p != nullptr;
		}

		auto_pointer get() const {
			return *this;
		}

		template<class T> T* get() const {
			return (T*)this->p;
		}
		
		template<class T> T* get_raw() const {
			return (T*)this->p;
		}

		template<class T> operator T* () const {
			return reinterpret_cast<T*>(p);
		}
	};

	/*
	 *  basic_memory_pointer
	 *      A memory pointer class that is capable of many operations, including address translation
	 *      MemTranslator is the translator functor
	 */
	template<class MemTranslator> union basic_memory_pointer {
	protected:
		void* p;
		uintptr_t a;

		// Translates address p to the running executable pointer
		static auto_pointer memory_translate(void* p) {
			return auto_pointer(MemTranslator()(p));
		}

	public:
		basic_memory_pointer() : p(nullptr) {}
		basic_memory_pointer(nullptr_t) : p(nullptr) {}
		basic_memory_pointer(uintptr_t x) : a(x) {}
		basic_memory_pointer(const auto_pointer& x) : p(x.p) {}
		basic_memory_pointer(const basic_memory_pointer& rhs) : p(rhs.p) {}

		template<class T>
		basic_memory_pointer(T* x) : p((void*)x) {}

		// Gets the translated pointer (plus automatic casting to lhs)
		auto_pointer get() const {
			return memory_translate(p);
		}

		// Gets the translated pointer (casted to T*)
		template<class T> T* get() const {
			return get();
		}

		// Gets the raw pointer, without translation (casted to T*)
		template<class T> T* get_raw() const {
			return auto_pointer(p);
		}

		// This type can get assigned from void* and uintptr_t
		basic_memory_pointer& operator=(void* x) {
			return p = x, *this;
		}

		basic_memory_pointer& operator=(uintptr_t x) {
			return a = x, *this;
		}

		/* Arithmetic */
		basic_memory_pointer operator+(const basic_memory_pointer& rhs) const {
			return basic_memory_pointer(this->a + rhs.a);
		}

		basic_memory_pointer operator-(const basic_memory_pointer& rhs) const {
			return basic_memory_pointer(this->a - rhs.a);
		}

		basic_memory_pointer operator*(const basic_memory_pointer& rhs) const {
			return basic_memory_pointer(this->a * rhs.a);
		}

		basic_memory_pointer operator/(const basic_memory_pointer& rhs) const {
			return basic_memory_pointer(this->a / rhs.a);
		}

		/* Comparision */
		bool operator==(const basic_memory_pointer& rhs) const {
			return this->a == rhs.a;
		}

		bool operator!=(const basic_memory_pointer& rhs) const {
			return this->a != rhs.a;
		}

		bool operator<(const basic_memory_pointer& rhs) const {
			return this->a < rhs.a;
		}

		bool operator<=(const basic_memory_pointer& rhs) const {
			return this->a <= rhs.a;
		}

		bool operator>(const basic_memory_pointer& rhs) const {
			return this->a > rhs.a;
		}

		bool operator>=(const basic_memory_pointer& rhs) const {
			return this->a >= rhs.a;
		}

		bool is_null() const {
			return this->p == nullptr;
		}

		uintptr_t as_int() const {
			// does not perform translation
			return this->a;
		}

#if __cplusplus >= 201103L || _MSC_VER >= 1800  // MSVC 2013
		/* Conversion to other types */
		explicit operator uintptr_t() const
		{
			return this->a;
		}	// does not perform translation
		explicit operator bool() const
		{
			return this->p != nullptr;
		}
#else
		//operator bool() -------------- Causes casting problems because of implicitness, use !is_null()
		//{ return this->p != nullptr; }
#endif
	};

	// Typedefs including memory translator for the above type
	typedef basic_memory_pointer<address_manager::fn_mem_translator>       memory_pointer;
	typedef basic_memory_pointer<address_manager::fn_mem_translator_nop>   memory_pointer_raw;

	/*
	 *  memory_pointer_tr
	 *      Stores a basic_memory_pointer<Tr> as a raw pointer from translated pointer
	 */
	union memory_pointer_tr {
	protected:
		void* p;
		uintptr_t a;

	public:
		template<class Tr> memory_pointer_tr(const basic_memory_pointer<Tr>& ptr) : p(ptr.get()) {
			// Constructs from a basic_memory_pointer
		}      

		memory_pointer_tr(const auto_pointer& ptr) : p(ptr.p) {
			// Constructs from a auto_pointer, probably comming from basic_memory_pointer::get
		}  

		memory_pointer_tr(const memory_pointer_tr& rhs): p(rhs.p) {
			// Constructs from my own type, copy constructor
		} 
		
		auto_pointer get() {
			// Just to be method-compatible with basic_memory_pointer ...
			return auto_pointer(p);
		}
		
		template<class T> T* get() {
			return get();
		}

		template<class T> T* get_raw() {
			return get();
		}

		memory_pointer_tr operator+(const uintptr_t& rhs) const {
			return memory_pointer_raw(this->a + rhs);
		}

		memory_pointer_tr operator-(const uintptr_t& rhs) const {
			return memory_pointer_raw(this->a - rhs);
		}

		memory_pointer_tr operator*(const uintptr_t& rhs) const {
			return memory_pointer_raw(this->a * rhs);
		}

		memory_pointer_tr operator/(const uintptr_t& rhs) const {
			return memory_pointer_raw(this->a / rhs);
		}

		bool is_null() const {
			return this->p == nullptr;
		}

		uintptr_t as_int() const {
			return this->a;
		}
	};

	inline bool UnprotectMemory(memory_pointer_tr addr, size_t size, DWORD& out_oldprotect);
	inline bool ProtectMemory(memory_pointer_tr addr, size_t size, DWORD protection);

	/*
	 *  scoped_unprotect
	 *      RAII wrapper for UnprotectMemory
	 *      On construction unprotects the memory, on destruction reprotects the memory
	 */
	struct scoped_unprotect {
		memory_pointer_raw  addr;
		size_t              size;
		DWORD               dwOldProtect;
		bool                bUnprotected;

		scoped_unprotect(memory_pointer_tr addr, size_t size){
			if (size == 0) bUnprotected = false;
			else          bUnprotected = UnprotectMemory(this->addr = addr.get<void>(), this->size = size, dwOldProtect);
		}

		~scoped_unprotect() {
			if (bUnprotected) ProtectMemory(this->addr.get(), this->size, this->dwOldProtect);
		}
	};

	typedef basic_memory_pointer<address_manager::fn_mem_translator_nop>   memory_pointer_raw;

	inline uintptr_t GetRelativeOffset(memory_pointer_tr, memory_pointer_tr);
	inline memory_pointer_raw GetBranchDestination(memory_pointer_tr, bool);
	inline memory_pointer_raw MakeJMP(memory_pointer_tr, memory_pointer_raw, bool);
	inline memory_pointer_raw ReadRelativeOffset(memory_pointer_tr, size_t, bool vp);
	template<class T> inline T ReadMemory(memory_pointer_tr addr, bool vp = false);
	template<class T> inline memory_pointer_tr WriteMemory(memory_pointer_tr addr, T value, bool vp = false);
	inline void MakeRelativeOffset(memory_pointer_tr, memory_pointer_tr, size_t, bool);
	inline memory_pointer_raw GetAbsoluteOffset(int rel_value, memory_pointer_tr end_of_instruction);
	template<class T> inline T& WriteObject(memory_pointer_tr addr, const T& value, bool vp = false);
	template<class T> inline T& ReadObject(memory_pointer_tr addr, T& value, bool vp = false);
}