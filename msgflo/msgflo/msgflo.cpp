#include <time.h>
#include "debug.h"
#include "msgflo.h"

MsgFlo::MsgFlo() {
#ifdef _DEBUG
  attachDebugConsole();
#endif
  trace();

  string dllName = "paho-mqtt3c.dll";
  char pExePath[256];

  GetModuleFileName(NULL, pExePath, sizeof(pExePath));

  string exePath = pExePath;
  string dllPath = exePath.substr(0, exePath.find_last_of("\\") + 1);
  dllPath += "Plugins\\";
  dllPath += dllName;

  printf("Paho path is: %s", pExePath);

  pPaho_ = new Paho(dllPath.c_str());
}

MsgFlo::~MsgFlo() {  
  if(pPaho_)
    delete pPaho_;

  trace();

#ifdef _DEBUG
  detachDebugConsole();
#endif
}

void MsgFlo::attachDebugConsole() {
  if (!isConsoleAttached_) {
    AllocConsole();

    CONSOLE_SCREEN_BUFFER_INFO coninfo;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
    coninfo.dwSize.Y = 500;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

    freopen_s(&pDebugStream_, "CONOUT$", "w", stdout);

    isConsoleAttached_ = true;
  }
}

void MsgFlo::detachDebugConsole() {
  if (isConsoleAttached_) {
    fclose(pDebugStream_);

    // TODO: set stdout to some empty stream, which does not crash when calling printf after this!?

    FreeConsole();

    isConsoleAttached_ = false;
  }
}

void MsgFlo::onFirstCycle() {
  trace();

  brokerHostname_ = "tcp://c-beam:1883";
  topic_ = "werkstatt/c_nancy_beta/";

  pPaho_->connect(brokerHostname_.c_str(), "c_nancy");
  pPaho_->publish((topic_ + "running").c_str(), "true", 4, 1, true);

  lastTick_ = clock();
  lastTick2_ = clock();
}

void MsgFlo::onTick() {  
  long tick = clock();
  long diffSeconds = (tick - lastTick_) / 1000;
  long diffSeconds2 = (tick - lastTick2_) / 1000;

  if (diffSeconds2 == 1) {
    // TODO: add GetField() callback to c# app

    double x = tick; // UC.Getfield(true, 226);
    double y = tick; // UC.Getfield(true, 227);
    double z = tick; // UC.Getfield(true, 228);
    double a = tick; // UC.Getfield(true, 229);
    double b = tick; // UC.Getfield(true, 230);
    double c = tick; // UC.Getfield(true, 231);

    char pStatus[512];
    snprintf(pStatus, sizeof(pStatus), R"({"X": %f, "Y": %f, "Z": %f, "A": %f, "B": %f, "C": %f})", x, y, z, a, b, c);

    pPaho_->publish((topic_ + "status").c_str(), pStatus, strlen(pStatus), 1, false);
    lastTick2_ = clock();
  }

  if (diffSeconds == 60) {
    string discovery = R"({"protocol": "discovery", "command": "participant", "payload": " \
                        "{"component": "c-base/c_nancy", "label": "CNC mill status", \ 
                        "icon": "scissors", "inports": [], "outports": [{"id": "running", \
                        "type": "boolean", "queue": "werkstatt/c_nancy/running"}], "role": "c_nancy", "id": "c_nancy"}})";

    pPaho_->publish("fbp", discovery.c_str(), discovery.length(), 1, true);
    lastTick_ = clock();
  }
}

void MsgFlo::onShutdown() {
  trace();

  pPaho_->publish((topic_ + "running").c_str(), "false", 5, 1, true);
  pPaho_->disconnect();  
}
