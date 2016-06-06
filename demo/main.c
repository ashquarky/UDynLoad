#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/fs_functions.h"
#include "dynamic_libs/gx2_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/padscore_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/ax_functions.h"
#include "fs/fs_utils.h"
#include "fs/sd_fat_devoptab.h"
#include "system/memory.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include "common/common.h"

#include "UDynLoad.h"
#include "kernel.h"

// unsigned char* screenBuffer;  // Change to hard-coded offset to avoid screen shifting
int screen_buf0_size = 0;
int screen_buf1_size = 0;

void printstr(int line, char* string);

void bad() {
	OSFatal("bad");
}

void exception_disassembly_helper(char *fmt, int addr,int opcode, char* s)
{
    char* *store = (char**)0x1ab5d140;
    char *buffer = (char *)store[0];
    store[0] += __os_snprintf(buffer,512,fmt, addr,opcode,s);
}
unsigned char exception_handler(void* contextIn) {
	
	//Temporary hacky fix, please ignore me.
	unsigned int coreinit_handle;
	OSDynLoad_Acquire("coreinit.rpl", &coreinit_handle);
	void (*DisassemblePPCRange)(void *start, void *end, void *printf_func, int *find_symbol_func, int flags);
	OSDynLoad_FindExport(coreinit_handle, 0, "DisassemblePPCRange", &DisassemblePPCRange);  
	
	int* context = (int*)contextIn;
	
	char buf2[512];
	int* store = (int*)0x1AB5D140;
	store[0] = (int)buf2;
	
	DisassemblePPCRange((void*)context[38]-0x5, (void*)context[38]+0x30, (void*)exception_disassembly_helper, 0, 0);
	
	int ancientSP = 0xDEADCAFE;
	int* crashSP = (int*)context[3];
	
	ancientSP = *(crashSP - 0x16); //where we're storing the old SP
	
	char buf[1024];
	__os_snprintf(buf, 1024, "r0:%08X  SP:%08X  r2:%08X  r3:%08X r9:%08X\n r10:%08X r11:%08X r30:%08X r31:%08X\nlr:%08X slr:%08X  PC:%08X\n%s", context[2], context[3], context[4], context[5], context[11], context[12], context[13], context[32], context[33], context[35], ancientSP, context[38], buf2);
	
	OSScreenClearBufferEx(0, 0);
    OSScreenClearBufferEx(1, 0);
	
	DCFlushRange((void*)0xF4000000, screen_buf0_size);
    DCFlushRange((void*)(0xF4000000 + screen_buf0_size), screen_buf1_size);
	OSScreenFlipBuffersEx(0);
    OSScreenFlipBuffersEx(1);
	
	OSScreenClearBufferEx(0, 0);
    OSScreenClearBufferEx(1, 0);
	
	printstr(0, buf);
	
	int vpadError = -1;
    VPADData vpad;

    while(1)
    {
        VPADRead(0, &vpad, 1, &vpadError);

        if(vpadError == 0 && ((vpad.btns_d | vpad.btns_h) & VPAD_BUTTON_HOME))
            break;

		usleep(50000);
    }
	
	printstr(12, "exiting...");
	__Exit();
	
	return 0;
}

/* Convenience! */
void printstr(int line, char* string) {
	OSScreenPutFontEx(0, 0, line, string);
	OSScreenPutFontEx(1, 0, line, string);
	DCFlushRange((void*)0xF4000000, screen_buf0_size);
    DCFlushRange((void*)(0xF4000000 + screen_buf0_size), screen_buf1_size);
    OSScreenFlipBuffersEx(0);
    OSScreenFlipBuffersEx(1);
	OSScreenPutFontEx(0, 0, line, string);
	OSScreenPutFontEx(1, 0, line, string);
	DCFlushRange((void*)0xF4000000, screen_buf0_size);
    DCFlushRange((void*)(0xF4000000 + screen_buf0_size), screen_buf1_size);
	OSScreenFlipBuffersEx(0);
    OSScreenFlipBuffersEx(1);
}

/* Entry point */
int Menu_Main(void)
{
    //!*******************************************************************
    //!                   Initialize function pointers                   *
    //!*******************************************************************
    //! do OS (for acquire) and sockets first so we got logging
    InitOSFunctionPointers();
    InitSocketFunctionPointers();

    log_init("192.168.178.3");
    log_print("Starting launcher\n");

    InitFSFunctionPointers();
    InitVPadFunctionPointers();

    log_print("Function exports loaded\n");

    //!*******************************************************************
    //!                    Initialize heap memory                        *
    //!*******************************************************************
    log_print("Initialize memory management\n");
    //! We don't need bucket and MEM1 memory so no need to initialize
    memoryInitialize();
	
	OSSetExceptionCallback(2, &exception_handler);
	OSSetExceptionCallback(3, &exception_handler);
	OSSetExceptionCallback(6, &exception_handler);
	
    //!*******************************************************************
    //!                        Initialize FS                             *
    //!*******************************************************************
    log_printf("Mount SD partition\n");
    mount_sd_fat("sd");

    VPADInit();

    // Init screen and screen buffers
    OSScreenInit();
    screen_buf0_size = OSScreenGetBufferSizeEx(0);
    screen_buf1_size = OSScreenGetBufferSizeEx(1);

    // screenBuffer = MEM1_alloc(screen_buf0_size + screen_buf1_size, 0x40);  // Change to hard-coded offset to avoid screen shifting

    OSScreenSetBufferEx(0, (void *)0xF4000000);  // Change to hard-coded offset to avoid screen shifting
    OSScreenSetBufferEx(1, ((void *)0xF4000000 + screen_buf0_size));  // Change to hard-coded offset to avoid screen shifting

    OSScreenEnableEx(0, 1);
    OSScreenEnableEx(1, 1);

    // Clear screens
    OSScreenClearBufferEx(0, 0);
    OSScreenClearBufferEx(1, 0);

	printstr(0, "UDynLoad - An ELF Dynamic Loader for the Wii U");
	printstr(1, "Demo from Dimok's hello world ELF - tinyurl.com/DHelloElf");
	
    unsigned char* elfFileBad = 0;
	unsigned int elfFileSize = 0;
	
	int result = LoadFileToMem("sd://lib.elf", &elfFileBad, &elfFileSize);
	
	char buf[255];
	__os_snprintf(buf, 255, "FS result %d", result);
	printstr(2, buf);
	
	if (elfFileBad != 0) {
		InjectSyscall36((unsigned int)injectBAT);
		RunSyscall36();
		
		unsigned int* elfFile = 0x0; //DBATted to 0x30000000
		
		__os_snprintf(buf, 255, "Copying %d bytes from 0x%08X to 0x%08X...", elfFileSize, elfFileBad, elfFile);
		printstr(3, buf);
		
		memcpy(elfFile, elfFileBad, elfFileSize);
		
		printstr(4, "Done!");
		
		__os_snprintf(buf, 255, "ELF magic: %X %c %c %c", *(elfFile), *(elfFile + 1), *(elfFile + 2), *(elfFile + 3));
		printstr(5, buf);
		
		int check_elf_result = UDynLoad_CheckELF(elfFile);
		__os_snprintf(buf, 255, "ELF validator returned %d", check_elf_result);
		printstr(6, buf);
		
		int (*anotherFunction)(int i);
		
		int returned = UDynLoad_FindExport(elfFile, 0, "setIntegerOne", &anotherFunction);
		
		__os_snprintf(buf, 255, "Got function 0x%08X, ret %d", anotherFunction, returned);
		printstr(7, buf);
	
		__os_snprintf(buf, 255, "This is here to screw with the registers for a bit 0x%X %X %X %X %X", 11, 12, 13, 14, 15);
		printstr(8, buf);
		
		int vpadError = -1;
    VPADData vpad;

    while(1)
    {
        VPADRead(0, &vpad, 1, &vpadError);

        if(vpadError == 0 && ((vpad.btns_d) & VPAD_BUTTON_HOME))
            break;

		usleep(50000);
    }
		
		anotherFunction(223);
		
		int ret = 42;
		
		__os_snprintf(buf, 255, "Returned 0x%08X", ret);
		printstr(9, buf);
	}
	
	printstr(10, "Removing DBAT...");
	InjectSyscall36((unsigned int)clearBAT);
	RunSyscall36();
	
	printstr(11, "Done! Press HOME to quit.");
	
    int vpadError = -1;
    VPADData vpad;

    while(1)
    {
        VPADRead(0, &vpad, 1, &vpadError);

        if(vpadError == 0 && ((vpad.btns_d) & VPAD_BUTTON_HOME))
            break;

		usleep(50000);
    }

	// MEM1_free(screenBuffer);  // Change to hard-coded offset to avoid screen shifting
	// screenBuffer = NULL;  // Change to hard-coded offset to avoid screen shifting

    //!*******************************************************************
    //!                    Enter main application                        *
    //!*******************************************************************

    unmount_sd_fat("sd");
    memoryRelease();

    return EXIT_SUCCESS;
}

