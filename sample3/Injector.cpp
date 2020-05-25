#include "Injector.h"

namespace Injector {
	/*
	 *  MakeJMP
	 *      Creates a JMP instruction at address @at that jumps into address @dest
	 *      If there was already a branch instruction there, returns the previosly destination of the branch
	 *      全体では14バイト使用する
	 */
	inline memory_pointer_raw MakeJMP(memory_pointer_tr at, memory_pointer_raw dest, bool vp = true) {
		auto p = GetBranchDestination(at, vp);

		auto offset = GetRelativeOffset(dest, at + 1 + 4);

		if (offset > 0x7FFFFFFF) {
			//WriteMemory<uint8_t>(at, 0x48, vp); // REX.w ‐ 1=オペランドサイズを64ビットにする。
			WriteMemory<uint8_t>(at, 0xFF, vp); // operand①
			// Mod/R: [RIP + disp32]を意味する
			//        Mod: 00b : レジスター+レジスター
			//        reg: 100b : operand② ①と②の組み合わせでjmpをnearで実施になる
			//        r/m: 101b : x86だとdisp32のみだったがx64ではRIP（この命令の終わりのアドレス）を意味
			WriteMemory<uint8_t>(at + 1, 0x25, vp);
			// displacement 32には0を入れてRIPのすぐ後ろを見るようにする
			WriteMemory<uint32_t>(at + 2, 0x0, vp);
			// jmp先のアドレスを書く
			WriteMemory<memory_pointer_raw>(at + 6, dest, vp);
		}
		else {
			WriteMemory<uint8_t>(at, 0xE9, vp);
			MakeRelativeOffset(at + 1, dest, 4, vp);
		}

		return p;
	}

	/*
	 *  GetBranchDestination
	 *      Gets the destination of a branch instruction at address @at
	 *      *** Works only with JMP and CALL for now ***
	 */
	inline memory_pointer_raw GetBranchDestination(memory_pointer_tr at, bool vp = true) {
		switch (ReadMemory<uint8_t>(at, vp)) {


		case 0x48:
		case 0x4C:
			switch (ReadMemory<uint8_t>(at + 1, vp)) {
			case 0x8B: // mov
			case 0x8D: // lea
				switch (ReadMemory<uint8_t>(at + 2, vp)) {
				case 0x0D:
				case 0x15:
					return ReadRelativeOffset(at + 3, 4, vp);
				}
				break;
			}
			break;

			// We need to handle other instructions (and prefixes) later...
		case 0xE8:	// call rel
		case 0xE9:	// jmp rel
			return ReadRelativeOffset(at + 1, 4, vp);

		case 0xFF:
		case 0x0F:
			switch (ReadMemory<uint8_t>(at + 1, vp)) {
			case 0x15:  // call dword ptr [addr]
			case 0x25:  // jmp dword ptr [addr]
			case 0x85:  // jne
			case 0x84:  // jz
				auto a = ReadRelativeOffset(at + 2, 4, vp);
				return a;
			}
			break;
		}
		return nullptr;
	}

	/*
	 *  GetRelativeOffset
	 *      Gets relative offset based on absolute address @abs_value for instruction that ends at @end_of_instruction
	 */
	inline uintptr_t GetRelativeOffset(memory_pointer_tr abs_value, memory_pointer_tr end_of_instruction) {
		return uintptr_t(abs_value.get<char>() - end_of_instruction.get<char>());
	}

	/*
	 *  ReadRelativeOffset
	 *      Reads relative offset from address @at
	 */
	inline memory_pointer_raw ReadRelativeOffset(memory_pointer_tr at, size_t sizeof_addr = 4, bool vp = true) {
		switch (sizeof_addr)
		{
		case 1: return (GetAbsoluteOffset(ReadMemory<int8_t>(at, vp), at + sizeof_addr));
		case 2: return (GetAbsoluteOffset(ReadMemory<int16_t>(at, vp), at + sizeof_addr));
		case 4: return (GetAbsoluteOffset(ReadMemory<int32_t>(at, vp), at + sizeof_addr));
		}
		return nullptr;
	}

	/*
	 *  ReadMemory
	 *      Reads the object type T at address @addr
	 *      Does memory unprotection if @vp is true
	 */
	template<class T> inline T ReadMemory(memory_pointer_tr addr, bool vp) {
		T value;
		return ReadObject(addr, value, vp);
	}

	/*
	 *  WriteMemory
	 *      Writes the object of type T into the address @addr
	 *      Does memory unprotection if @vp is true
	 */
	template<class T> inline memory_pointer_tr WriteMemory(memory_pointer_tr addr, T value, bool vp) {
		WriteObject(addr, value, vp);
		return addr + sizeof(value);
	}

	/*
	 *  MakeRelativeOffset
	 *      Writes relative offset into @at based on absolute destination @dest
	 */
	inline void MakeRelativeOffset(memory_pointer_tr at, memory_pointer_tr dest, size_t sizeof_addr = 4, bool vp = true){
		switch (sizeof_addr) {
		case 1: WriteMemory<int8_t>(at, static_cast<int8_t> (GetRelativeOffset(dest, at + sizeof_addr)), vp);
		case 2: WriteMemory<int16_t>(at, static_cast<int16_t>(GetRelativeOffset(dest, at + sizeof_addr)), vp);
		case 4: WriteMemory<int32_t>(at, static_cast<int32_t>(GetRelativeOffset(dest, at + sizeof_addr)), vp);
		}
	}

	/*
	 *  GetAbsoluteOffset
	 *      Gets absolute address based on relative offset @rel_value from instruction that ends at @end_of_instruction
	 */
	inline memory_pointer_raw GetAbsoluteOffset(int rel_value, memory_pointer_tr end_of_instruction)
	{
		return end_of_instruction.get<char>() + rel_value;
	}

	/*
	 *  WriteObject
	 *      Assigns the object @value into the same object type at @addr
	 *      Does memory unprotection if @vp is true
	 */
	template<class T> inline T& WriteObject(memory_pointer_tr addr, const T& value, bool vp) {
		scoped_unprotect xprotect(addr, vp ? sizeof(value) : 0);
		return (*addr.get<T>() = value);
	}

	/*
	 *  UnprotectMemory
	 *      Unprotect the memory at @addr with size @size so it have all accesses (execute, read and write)
	 *      Returns the old protection to out_oldprotect
	 */
	inline bool UnprotectMemory(memory_pointer_tr addr, size_t size, DWORD& out_oldprotect) {
		return mprotect(addr.get(), size, PROT_WRITE | PROT_READ);
	}

	/*
	 *  ProtectMemory
	 *      Makes the address @addr have a protection of @protection
	 */
	inline bool ProtectMemory(memory_pointer_tr addr, size_t size, DWORD protection){
		return mprotect(addr.get(), size, PROT_WRITE) != 0;
	}

	/*
	 *  ReadObject
	 *      Assigns the object @value with the value of the same object type at @addr
	 *      Does memory unprotection if @vp is true
	 */
	template<class T> inline T& ReadObject(memory_pointer_tr addr, T& value, bool vp){
		scoped_unprotect xprotect(addr, vp ? sizeof(value) : 0);
		return (value = *addr.get<T>());
	}
}