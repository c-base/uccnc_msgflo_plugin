#include <time.h>
#include <limits.h>
#include "json.hpp"
#include "debug.h"
#include "msgflo.h"

using json = nlohmann::json;

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

void MsgFlo::setCallBacks(GetFieldDoubleFunc* pGetFieldDouble, PluginInterfaceEntry uc) {
  trace();

  UC = uc;
}

void MsgFlo::onFirstCycle() {
  trace();

  string topic = MQTT_BASE_TOPIC;
  topic += "running";

  pPaho_->connect(MQTT_BROKER_HOSTNAME, MQTT_CLIENT_ID);
  pPaho_->publish(topic.c_str(), "true", 4, 1, true);

  lastTick_ = clock();
  lastTick2_ = clock();
}

void MsgFlo::onTick() {
  long tick = clock();
  long diffSeconds = (tick - lastTick_) / 1000;
  long diffMs2 = (tick - lastTick2_);

  if (diffMs2 >= 250) {
    position_.x = UC.pGetFieldDouble(true, 226) / 10000.0;
    position_.y = UC.pGetFieldDouble(true, 227) / 10000.0;
    position_.z = UC.pGetFieldDouble(true, 228) / 10000.0;
    position_.a = UC.pGetFieldDouble(true, 229) / 10000.0;
    position_.b = UC.pGetFieldDouble(true, 230) / 10000.0;
    position_.c = UC.pGetFieldDouble(true, 231) / 10000.0;

    if (UC.pIsMoving()) {
      json status;
      status["X"] = position_.x;
      status["Y"] = position_.y;
      status["Z"] = position_.z;
      status["A"] = position_.a;
      status["B"] = position_.b;
      status["C"] = position_.c;

      string statusStr = status.dump();
      pPaho_->publish(baseTopic_ + "position", statusStr.c_str(), statusStr.length(), 1, false);
    }

    lastTick2_ = clock();
  }

  if (diffSeconds >= 60) {
    json root;
    root["protocol"] = "discovery";
    root["command"] = "participant";

    json payload;
    payload["component"] = "c-base/c_nancy";
    payload["label"] = "CNC mill status";
    payload["icon"] = "scissors";
    payload["inports"] = json::array();

    json outports = json::array();
    json runningPort;
    runningPort["id"] = "running";
    runningPort["type"] = "boolean";
    runningPort["queue"] = baseTopic_ + "running";

    json positionPort;
    positionPort["id"] = "position";
    positionPort["type"] = "object";
    positionPort["queue"] = baseTopic_ + "position";

    outports.push_back(runningPort);
    outports.push_back(positionPort);

    root["outports"] = outports;
    root["role"] = "c_nancy";
    root["id"] = "c_nancy";
    root["payload"] = payload;

    printf("payload object:\n---\n%s\n---\n", root.dump(4).c_str());

    string discoveryStr = root.dump();
    pPaho_->publish("fbp", discoveryStr.c_str(), discoveryStr.length(), 1, true);

    lastTick_ = clock();
  }
}

void MsgFlo::onShutdown() {
  trace();

  pPaho_->publish(baseTopic_ + "running", "false", 5, 1, true);
  pPaho_->disconnect();
}

void MsgFlo::buttonPressEvent(int buttonNumber, bool onScreen) {
  // trace();

  if (onScreen) {
    if (buttonNumber == 128) {
      // TODO: implement

    }
  }
}

void MsgFlo::textFieldClickEvent(int labelNumber, bool isMainScreen) {
  trace();

  if (isMainScreen) {
    if (labelNumber == 1000) {
      // TODO: implement
    }
  }
}

void MsgFlo::textFieldTextTypedEvent(int labelNumber, bool isMainScreen, const char* pText) {
  trace();

  if (isMainScreen) {
    if (labelNumber == 1000) {
      // TODO: implement
    }
  }
}

void MsgFlo::getPropertiesEvent(char* pAuthor, char* pPluginName, char* pPluginVersion) {
  trace();

  strcpy_s(pAuthor, 256, AUTHOR);
  strcpy_s(pPluginName, 256, PLUGIN_NAME);
  strcpy_s(pPluginVersion, 256, PLUGIN_VERSION);
}
