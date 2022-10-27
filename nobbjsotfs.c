/*
 *    build cmd with mingw64:
 * 		gcc -m64 -Wall nobbjsotfs.c -shared -o DINPUT8.dll -l Psapi
 */

/*
		 This DLL implements the bbj mod originally named in CE:
			"Baby Jump fix (only one can be active at a time)"
*/

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <psapi.h>
#include <processthreadsapi.h>

//#define NOLOGO

/*
 *    This patching mechanism is taken from:
 *        https://github.com/bladecoding/DarkSouls3RemoveIntroScreens/blob/master/SoulsSkipIntroScreen/dllmain.cpp
 */
struct patch {
    DWORD rel_addr;
    DWORD size;
    char patch[50];
    char orig[50];
};

typedef HRESULT (WINAPI *dinp8crt_t)(HINSTANCE, DWORD, REFIID,
				     LPVOID *, LPUNKNOWN);
dinp8crt_t oDirectInput8Create;

__attribute__ ((dllexport))
HRESULT WINAPI DirectInput8Create(HINSTANCE inst, DWORD ver, REFIID id,
				  LPVOID *pout, LPUNKNOWN outer)
{
    return oDirectInput8Create(inst, ver, id, pout, outer);
}

void setup_d8proxy(void)
{
    char syspath[320];
    GetSystemDirectoryA(syspath, 320);
    strcat(syspath, "\\dinput8.dll");
    HMODULE mod = LoadLibraryA(syspath);
    oDirectInput8Create = (dinp8crt_t)GetProcAddress(mod, "DirectInput8Create");
}

void attach_hook(void)
{ 	
	void *module_addr = GetModuleHandle(NULL);
	MODULEINFO moduleInfo;
	GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL), &moduleInfo, sizeof(moduleInfo));
	
	//// TESTING (GetModuleSize --> DS2 Version)
	//FILE* out_file = fopen("moduleinfo.txt", "w");						// write only 
	//fprintf(out_file, "Module size: %ld\n", moduleInfo.SizeOfImage);		// write to file 
	//fclose(out_file);
	
	// Declare address variables:
	DWORD basea_offset;
	DWORD nologo_offset;
	DWORD jumpfcn_offset;

	// Switch version / Populate addresses:
	if (moduleInfo.SizeOfImage == 34299904) {
		// Unpatched game:
		basea_offset = 0x160B8D0;
		nologo_offset = 0x1604DFA;
		jumpfcn_offset = 0x37B4BC;
	}
	else if (moduleInfo.SizeOfImage == 30892032) {
		// Online patch:
		basea_offset = 0x16148F0;
		nologo_offset = 0x160DE1A;
		jumpfcn_offset = 0x0381E1C;
	}
	else {
		return; // unimplemented version
	}

	//////////////////////////////////////////
	//				NOLOGO Mod:
#ifdef NOLOGO
	*(byte*)(module_addr + nologo_offset) = 0x01;
#endif
	//////////////////////////////////////////
	

	// Setup the (version-specific) basepointer:
	void * baseA = module_addr + basea_offset;
	
	struct patch jump_patch = 
	    {
			jumpfcn_offset, 										// rel_addr  
			16,														// size (bytes)

			// patch (inject):
			{0x49, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 	// mov r10, addr_xxxxxxxx
			 0x41, 0xFF, 0xE2,												// [10] jmp r10
			 0x90, 0x90, 0x90 												// [13] nop filler
			}, 
			
			// orig:
			{0xF3, 0x0F, 0x58, 0xC8, 				// addss xmm1,xmm0
			 0x0F, 0xC6, 0xC9, 0x00,            	// shufps xmm1,xmm1,00
			 0x0F, 0x51, 0xC1,     					// sqrtps xmm0,xmm1
			 0x0F, 0x29, 0x44, 0x24, 0x20 			// movaps [rsp+20],xmm0
			},       	
	    };
	
	// "NoBabyJ (full jump length)" code
	static char bbjcode_bytes[] =
	{
		// orig:
		0xF3, 0x0F, 0x58, 0xC8, 									// addss  xmm1,xmm0
		0x0F, 0xC6, 0xC9, 0x00,            							// shufps xmm1,xmm1,00
		0x0F, 0x51, 0xC1,     										// sqrtps xmm0,xmm1

		// newmem:
		0x50, 														// [11] push rax
		0x48, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // [12] mov rax,[baseA]
		0x48, 0x8B, 0x80, 0xD0, 0x00, 0x00, 0x00, 					// [22] mov rax,[rax+D0]
		0x48, 0x8B, 0x80, 0xF8, 0x00, 0x00, 0x00, 					// [29] mov rax,[rax+F8]
		0x48, 0x8B, 0x80, 0xF0, 0x00, 0x00, 0x00, 					// [36] mov rax,[rax+F0]
		0xF3, 0x0F, 0x10, 0x80, 0x98, 0x37, 0x00, 0x00, 			// [43] movss xmm0,[rax+3798]
		0x58, 														// [51] pop rax

		// orig:
		0x0F, 0x29, 0x44, 0x24, 0x20, 								// [52] movaps [rsp+20], xmm0

		//return to orig (64bit jmp)
		0x49, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // [57] mov r10, addr_xxxxxxxx
		0x41, 0xFF, 0xE2, 											// jmp r10 
	};

	// Update patch addresses:
	uint64_t addr_bbjcode = (uint64_t) bbjcode_bytes;
	memcpy(&jump_patch.patch[2], &addr_bbjcode, sizeof(addr_bbjcode)); 		// jmp fw
	
	// Fill in addresses in bbjcode:
	memcpy(&bbjcode_bytes[14],&baseA,sizeof(baseA)); 				// mov rax,[baseA]

	// Fix inject return address
	void* jumpinjaddr = module_addr + jump_patch.rel_addr;
	void* jumpinj_ret = jumpinjaddr + jump_patch.size;
	memcpy(&bbjcode_bytes[59],&jumpinj_ret,sizeof(jumpinj_ret)); 	// return to jumpinject (+offset)
	

	// Make bbjcode_bytes executable:
	DWORD op;
	VirtualProtect(bbjcode_bytes, sizeof(bbjcode_bytes),
		PAGE_EXECUTE_READWRITE, &op);

	// Edit memory:
	DWORD size = jump_patch.size;
	if (memcmp(jumpinjaddr, jump_patch.orig, size) == 0) {
		DWORD old;
		VirtualProtect(jumpinjaddr, size, PAGE_EXECUTE_READWRITE, &old);
		memcpy(jumpinjaddr, jump_patch.patch, size);
		VirtualProtect(jumpinjaddr, size, old, &old);
	}
	
}

BOOL APIENTRY DllMain(HMODULE mod, DWORD reason,
		      LPVOID res)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH:
	setup_d8proxy();
	attach_hook();
	break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
	break;
    }
    return TRUE;
}
