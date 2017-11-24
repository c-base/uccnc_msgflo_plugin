#include <windows.h>

#include "msgflo.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
      MsgFlo::create();
      break;

    case DLL_THREAD_ATTACH:
      break;

    case DLL_THREAD_DETACH:
      break;

    case DLL_PROCESS_DETACH:
      MsgFlo::destroy();

      break;
    }

    return TRUE;
}

extern "C" __declspec(dllexport)
void __cdecl empty() {
  MsgFlo::getInstance()->empty();
}

extern "C" __declspec(dllexport)
void __cdecl _string(const char* pString) {
  MsgFlo::getInstance()->string(pString);
}

extern "C" __declspec(dllexport)
void __cdecl stringAndNumber(const char* pString, int num) {
  MsgFlo::getInstance()->stringAndNumber(pString, num);
}

extern "C" __declspec(dllexport)
int __cdecl stringAndNumberWithIntRet(const char* pString, int num) {
  return MsgFlo::getInstance()->stringAndNumberWithIntRet(pString, num);
}
