/* UDynLoad - An ELF dynamic loader for Wii U
 * https://github.com/QuarkTheAwesome/UDynLoad
 *
 * UDynLoad.c - main code
 * Please see UDynLoad.h for documentation
 */

#include "UDynLoad.h"
#include "string.h"
#include "elf_abi.h"
 
int UDynLoad_CheckELF(void* elf) {
	Elf32_Ehdr* elfHeader = (Elf32_Ehdr*)elf;
	
	//Is file ELF?
	if (!IS_ELF (*elfHeader)) {
		return UDYNLOAD_ELF_NOT_ELF;
	}
	/* //Correction: We DO want non-executable elves.
	if (elfHeader->e_type != ET_EXEC) {
		return UDYNLOAD_ELF_NOT_EXEC;
	}*/
	//Is file for PowerPC?
	if (elfHeader->e_machine != EM_PPC) {
		return UDYNLOAD_ELF_NOT_PPC;
	}
	
	for (int i = 0; i < elfHeader->e_shnum; i++) {
		//section header file offset + offset of file in memory + (section header # * size of section header)
		Elf32_Shdr* sectionHeader = (Elf32_Shdr*)(elfHeader->e_shoff + elf + (i * sizeof(Elf32_Shdr)));
		//Is section a .dynsym?
		if (sectionHeader->sh_type == SHT_DYNSYM) {
			return UDYNLOAD_ELF_OK;
		}
	}
	
	//If we get to here, there is no .dynsym
	return UDYNLOAD_ELF_NO_DYNSYM;
}

int UDynLoad_FindExport(void* elf, int isdata, const char* symbolName, void* address) {
	//Get ELF header
	Elf32_Ehdr* elfHeader = (Elf32_Ehdr*)elf;
	Elf32_Shdr* dynSym = 0;
	Elf32_Shdr* dynStr = 0;
	
	for (unsigned int i = 0; i < elfHeader->e_shnum; i++) {
		Elf32_Shdr* sectionHeader = (Elf32_Shdr*)(elfHeader->e_shoff + elf + (i * sizeof(Elf32_Shdr)));
		if (sectionHeader->sh_type == SHT_DYNSYM) {
			dynSym = sectionHeader;
		} else if (sectionHeader->sh_type == SHT_STRTAB) {
			//AFAIK this is the only thing seperating .dynstr and .strtab/.shstrtab. Let me know if I'm wrong.
			if (sectionHeader->sh_flags == SHF_ALLOC) { 
				dynStr = sectionHeader;
			}
		}
	}
	
	if (!dynSym) {
		return UDYNLOAD_FIND_ERROR;
	} else if (!dynStr) {
		//TODO: add dynstr to CheckELF
		return UDYNLOAD_FIND_ERROR;
	}
	
	for (unsigned int i = 0; i < dynSym->sh_entsize; i++) {
		Elf32_Sym* symbol = (Elf32_Sym*)(elf + dynSym->sh_offset + (i * sizeof(Elf32_Sym)));
		//If symbol is function...
		if (ELF32_ST_TYPE(symbol->st_info) == STT_FUNC) {
			char* funcName = (elf + dynStr->sh_offset + symbol->st_name);
			//strcmp is weird, this actually evaluates to if these strings ARE the same.
			if (!strcmp(funcName, symbolName)) {
				*(int**)(address) = (int*)(symbol->st_value + elf);
				break;
			}
		}
		//We could theoretically elif for objects here (isdata) but I can't be bothered ;D
		//Tell me if you want this feature
	}
	
	return UDYNLOAD_FIND_NOT_FOUND;
}