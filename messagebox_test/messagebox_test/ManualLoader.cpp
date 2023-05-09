#include "ManualLoader.h"

PTEB getTeb(void)
{
	return NTCurrentTeb();
}

unsigned char toLowerC(unsigned char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 'a';
    else
        return c;
}

unsigned long djb2Hash(unsigned char* str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
    {
        hash = ((hash << 5) + hash) + toLowerC(c); // hash * 33 + c
    }
    return hash;
}

HMODULE getModHandle(WCHAR* ModuleName)
{
    /* Input validation */
    if (!ModuleName)
        return (HMODULE) -20;

    PPEB pPeb = getTeb()->ProcessEnvironmentBlock; // get the TEB
    //PPEB pPeb = (PPEB)__readgsqword(0x60); // get the PEB
    PPEB_LDR_DATA PebLdrData = { 0 };
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry = { 0 };
    PLIST_ENTRY ModuleList = { 0 }; // to store InMemoryOrderModuleList
    PLIST_ENTRY ForwardLink = { 0 }; // to store the Flink

    if (pPeb)
    {
        PebLdrData = pPeb->Ldr; // get the Ldr data from the _PEB

        if (PebLdrData)
        {
            ModuleList = &PebLdrData->InMemoryOrderModuleList; // get the InMemoryOrderModuleList 
            ForwardLink = ModuleList->Flink;

            while (ModuleList != ForwardLink)
            {
                /* Use the CONTAINING_RECORD macro to grab the LDR_DATA_TABLE_ENTRY from the current PLIST_ENTRY */
                LdrDataTableEntry = CONTAINING_RECORD(ForwardLink - 1, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
                if (LdrDataTableEntry)
                {
                    if (LdrDataTableEntry->BaseDllName.Buffer)
                    {
                        /* Use string hashing to check if the Dll entry matches the one we're looking for */
                        if (djb2Hash((unsigned char*)(LdrDataTableEntry->BaseDllName.Buffer)) == djb2Hash((unsigned char*)ModuleName))
                            return (HMODULE)LdrDataTableEntry->BaseAddress; // return the address of the DLL we found
                    }
                }
                ForwardLink = ForwardLink->Flink; // move down the list
            }
        }
    }

    return 0;
}

/*
    Rewrite of the GetProcAddress() function
*/
FARPROC getProcAddr(HMODULE module, LPCSTR lpName)
{
    /* Input validation */
    if (!module || !lpName)
        return (FARPROC) -1;

    /* Get the DOS Header and make sure it is g2g ('MZ') */
    IMAGE_DOS_HEADER* dosHdr = (IMAGE_DOS_HEADER*)module;
    if (dosHdr->e_magic != IMAGE_DOS_SIGNATURE)
        return (FARPROC) -2;

    /* Get the NT header and make sure it is g2g */
    IMAGE_NT_HEADERS* ntHdrs = (IMAGE_NT_HEADERS*)((BYTE*)module + dosHdr->e_lfanew);
    if (ntHdrs->Signature != IMAGE_NT_SIGNATURE)
        return (FARPROC) -3;

    /* Get the export directory of the dll and do some arithmetic to get the important tables (name, ordinal, function) */
    IMAGE_EXPORT_DIRECTORY* exportDir = (IMAGE_EXPORT_DIRECTORY*)((BYTE*)module + ntHdrs->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
    DWORD* nameTable = (DWORD*)((BYTE*)module + exportDir->AddressOfNames);
    WORD* ordinalTable = (WORD*)((BYTE*)module + exportDir->AddressOfNameOrdinals);
    DWORD* funcTable = (DWORD*)((BYTE*)module + exportDir->AddressOfFunctions);
    DWORD i;


    /* Iterate through the exports and check to see if any of them match the one we're looking for */
    for (i = 0; i < exportDir->NumberOfNames; i++)
    {
        LPCSTR name = (LPCSTR)((BYTE*)module + nameTable[i]);
        if (djb2Hash((unsigned char*)name) == djb2Hash((unsigned char*)lpName))
        {
            WORD ordinal = ordinalTable[i];
            DWORD rva = funcTable[ordinal];
            FARPROC ptr = (FARPROC)((BYTE*)module + rva);
            return ptr;
        }
    }

    return NULL; // ((void*)0); NULL
}
