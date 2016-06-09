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
		//Is section a .symtab?
		if (sectionHeader->sh_type == SHT_SYMTAB) {
			return UDYNLOAD_ELF_OK;
		}
	}
	
	//If we get to here, there is no .dynsym
	return UDYNLOAD_ELF_NO_SYMTAB;
}

int UDynLoad_FindExport(void* elf, int isdata, const char* symbolName, void* address) {
	*((void**)address) = 0; //Error checking for later
	//Get ELF header
	Elf32_Ehdr* elfHeader = (Elf32_Ehdr*)elf;
	Elf32_Shdr* symTab = 0;
	Elf32_Shdr* shstrTab = (Elf32_Shdr*)(elfHeader->e_shoff + elf + (elfHeader->e_shstrndx * sizeof(Elf32_Shdr)));
	Elf32_Shdr* strTab = 0;
	
	for (unsigned int i = 0; i < elfHeader->e_shnum; i++) {
		Elf32_Shdr* sectionHeader = (Elf32_Shdr*)(elfHeader->e_shoff + elf + (i * sizeof(Elf32_Shdr)));
		if (sectionHeader->sh_type == SHT_SYMTAB) {
			symTab = sectionHeader;
		} else if (sectionHeader->sh_type == SHT_STRTAB) {
			char* headerName = (elf + shstrTab->sh_offset + sectionHeader->sh_name);
			if (!strcmp(headerName, ELF_STRTAB)) {
				strTab = sectionHeader;
			}
		}
	}
	
	if (!symTab) {
		return UDYNLOAD_FIND_ERROR;
	} else if (!strTab) {
		//TODO: add dynstr to CheckELF
		return 69;
	}
	
	Elf32_Sym* symbol = 0;
	
	for (unsigned int i = 0; i <= symTab->sh_size; i++) {
		Elf32_Sym* s = (Elf32_Sym*)(elf + symTab->sh_offset + (i * sizeof(Elf32_Sym)));
		//If symbol is function...
		if (ELF32_ST_TYPE(s->st_info) == STT_FUNC) {
			char* funcName = (elf + strTab->sh_offset + s->st_name);
			//strcmp is weird, this actually evaluates to if these strings ARE the same.
			if (!strcmp(funcName, symbolName)) {
				symbol = s;
				break;
			}
		}
		//We could theoretically elif for objects here (isdata) but I can't be bothered ;D
		//Tell me if you want this feature
	}
	
	if (!symbol) {
		return 64;
	}
	
	*((void**)address) = (void*)(((Elf32_Shdr*)(elfHeader->e_shoff + elf + (symbol->st_shndx * sizeof(Elf32_Shdr))))->sh_offset + elf);
	if (!*(void**)address) {
		return UDYNLOAD_FIND_NOT_FOUND;
	}
	return UDYNLOAD_FIND_OK;
}