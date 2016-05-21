/* UDynLoad - An ELF dynamic loader for Wii U
 * https://github.com/QuarkTheAwesome/UDynLoad
 *
 * UDynLoad.c - main code
 * Please see UDynLoad.h for documentation
 */

#include "UDynLoad.h"
 
int UDynLoad_CheckELF(void* elf) {
	Elf32_Ehdr* elfHeader = (Elf32_Ehdr*)elf;
	
	//Is file ELF?
	if (!IS_ELF (*elfHeader)) {
		return UDYNLOAD_ELF_NOT_ELF;
	}
	//Is file executable?
	if (elfHeader->e_type != ET_EXEC) {
		return UDYNLOAD_ELF_NOT_EXEC;
	}
	//Is file for PowerPC?
	if (elfHeader->e_machine != EM_PPC) {
		return UDYNLOAD_ELF_NOT_PPC;
	}
	
	for (int i = 0; i < e_shnum; i++) {
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

int UDynLoad_FindExport(void* elf, int isdata, char* symbol, void* address) {
	
}