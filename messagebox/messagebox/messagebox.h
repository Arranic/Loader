// messagebox.h - Contains our message box wrapper function
#pragma once

#ifdef MESSAGEBOX_EXPORTS
#define MESSAGEBOX_API __declspec(dllexport)
#else
#define MESSAGEBOX_API __declspec(dllimport)
#endif

// the message box wrapper
extern "C" MESSAGEBOX_API void ShowMessageBox();