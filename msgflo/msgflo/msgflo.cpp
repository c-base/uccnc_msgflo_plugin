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

  pPaho_ = new Paho(dllPath);
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

void MsgFlo::setCallBacks(GetFieldDoubleFunc pGetFieldDouble) {
  trace();

  pGetFieldDouble_ = pGetFieldDouble;
}

void MsgFlo::onFirstCycle() {
  trace();

  string topic = MQTT_BASE_TOPIC;
  topic += "runnning";

  pPaho_->connect(MQTT_BROKER_HOSTNAME, MQTT_CLIENT_ID);
  pPaho_->publish(topic.c_str(), "true", 4, 1, true);

  lastTick_ = clock();
  lastTick2_ = clock();
}

void MsgFlo::onTick() {
  long tick = clock();
  long diffSeconds = (tick - lastTick_) / 1000;
  long diffSeconds2 = (tick - lastTick2_) / 1000;

  if (diffSeconds2 == 1) {
    double x = pGetFieldDouble_(true, 226) / 10000.0;
    double y = pGetFieldDouble_(true, 227) / 10000.0;
    double z = pGetFieldDouble_(true, 228) / 10000.0;
    double a = pGetFieldDouble_(true, 229) / 10000.0;
    double b = pGetFieldDouble_(true, 230) / 10000.0;
    double c = pGetFieldDouble_(true, 231) / 10000.0;

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

void MsgFlo::buttonPressEvent(int buttonNumber, bool onScreen) {
  if (onScreen) {
    if (buttonNumber == 128) {
      // TODO: implement

    }
  }
}

void MsgFlo::textFieldClickEvent(int labelnumber, bool Ismainscreen) {
  if (Ismainscreen) {
    if (labelnumber == 1000) {
      // TODO: implement
    }
  }
}

void MsgFlo::textFieldTextTypedEvent(int labelNumber, bool isMainScreen, const char* pText) {
  if (isMainScreen) {
    if (labelNumber == 1000) {
      // TODO: implement
    }
  }
}

void MsgFlo::getPropertiesEvent(char* pAuthor, char* pPluginName, char* pPluginVersion) {
  strcpy_s(pAuthor, 256, AUTHOR);
  strcpy_s(pPluginName, 256, PLUGIN_NAME);
  strcpy_s(pPluginVersion, 256, PLUGIN_VERSION);
}
