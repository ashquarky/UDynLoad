/* UDynLoad - An ELF dynamic loader for Wii U
 * https://github.com/QuarkTheAwesome/UDynLoad
 *
 * UDynLoad.h - main API header and documentation
 */

#ifndef _UDYNLOAD_H_
#define _UDYNLOAD_H_

#define UDYNLOAD_API_VERSION 1

/* Checks if a valid ELF is present at the given memory location.
 * Argument 1 (void* elf): Address of file to check.
 * Returns: One of the UDYNLOAD_ELF_* status codes below.
 */
int UDynLoad_CheckELF(void* elf);

#define UDYNLOAD_ELF_OK 0 //Good ELF loadable by UDynLoad.
#define UDYNLOAD_ELF_NOT_ELF 1 //No ELF magic (0x7F E L F) at memory location.
#define UDYNLOAD_ELF_NO_SYMTAB 2 //ELF does not appear to contain a .symtab section, thus is unreadable by UDynLoad.
#define UDYNLOAD_ELF_NOT_PPC 3 //ELF is not for PowerPC.
/*#define UDYNLOAD_ELF_NOT_EXEC 4 CORRECTION: Non-exectutable elves are what we DO want. */

/* Almost identical to coreinit's OSDynLoad_FindExport.
 * Argument 1 (void* elf): Address of ELF in memory. Make sure to UDynLoad_CheckELF first!
 * Argument 2 (int isdata): Ignored as of API v1. Just here for similarity to OSDynLoad.
 * Argument 3 (char* symbol): Name of function to find as string.
 * Argument 4 (void* address): Pointer to function pointer to point function to. (Just like OSDynLoad)
 * Returns: One of the UDYNLOAD_FIND_* status codes below.
 */
int UDynLoad_FindExport(void* elf, int isdata, const char* symbol, void* address);

#define UDYNLOAD_FIND_OK 0 //Found and loaded function successfully.
#define UDYNLOAD_FIND_NOT_FOUND 1 //Could not find function in .dymsym
#define UDYNLOAD_FIND_ERROR 2 //Other error (run CheckELF!)

#endif