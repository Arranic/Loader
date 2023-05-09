// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "messagebox.h"

void ShowMessageBox()
{
	printf("Inside DLL. Showing Message Box.\n");
	MessageBox(NULL, TEXT("Welcome to this DLL."), TEXT("DLL Export Test"), MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);
}