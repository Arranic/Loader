#include "ManualLoader.h"
#include <stdio.h>

typedef void (*pShowMessageBox)(void);
typedef HMODULE(WINAPI* LoadLibraryFunc)(LPCWSTR);
int main()
{
	HMODULE kernel, dll;
	FARPROC loader;
	pShowMessageBox func;

	kernel = GetModuleHandleW(L"kernel32.dll");
	if (kernel == NULL)
		return -10;

	printf("[+] Kernel handle loaded.\n");
	loader = getProcAddr(kernel, "LoadLibraryW");
	if (!loader)
		return -11;

	LoadLibraryFunc loadLibrary = (LoadLibraryFunc)loader;
	printf("[+] LoadLibraryW function loaded.\n");

	dll = loadLibrary(L"messagebox.dll");
	if (dll != NULL)
	{
		printf("[+] DLL found.\n");
		func = (pShowMessageBox)getProcAddr(dll, "ShowMessageBox");
		if (!func)
		{
			printf("[-] Unable to find function.\n");
			FreeLibrary(dll);
			exit(EXIT_FAILURE);
		}

		func();
		if (dll != NULL)
			FreeLibrary(dll);
		exit(EXIT_SUCCESS);
	}
	else
	{
		printf("[-] Unable to load messagebox.dll.\n");
		exit(EXIT_FAILURE);
	}
}