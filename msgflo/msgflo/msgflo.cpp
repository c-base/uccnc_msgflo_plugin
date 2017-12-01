﻿#include <time.h>
#include <limits.h>
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

  dbg("Paho path is: %s", pExePath);

  pPaho_ = new Paho(dllPath);
}

MsgFlo::~MsgFlo() {
  trace();

  if(pPaho_)
    delete pPaho_;

#ifdef _DEBUG
  detachDebugConsole();
#endif
}

void MsgFlo::mqttConect() {
  pPaho_->connect(MQTT_BROKER_HOSTNAME, MQTT_CLIENT_ID);
}

void MsgFlo::mqttDisconnect() {
  pPaho_->disconnect();
}

void MsgFlo::mqttPublish(string subTopic, const json& jsonObj, MsgRetain retain) {
  string jsonStr = jsonObj.dump();

  pPaho_->publish(baseTopic_ + subTopic, jsonStr.c_str(), jsonStr.length(), 1, retain == MsgRetain::Retain);
}

void MsgFlo::setCallBacks(PluginInterfaceEntry uc) {
  trace();

  UC = uc;
}

void MsgFlo::onFirstCycle() {
  trace();

  mqttConect();

  json j = true;
  mqttPublish("online", j, MsgRetain::Retain);
}

void MsgFlo::onTick() {
  long timeMs = clock();

  if (timeMs < 2000)
    return;

  handleDiscovery(timeMs);
  handlePositionState(timeMs);
  handleMillingState(timeMs);
  handleWorkTime(timeMs);
}

void MsgFlo::handleDiscovery(long timeMs) {
  // TODO: only update every minute:
  static long lastTick = 0;

  if ((timeMs - lastTick < 60 * 1000) && lastTick != 0)
    return;

  dbg("MsgFlo::handleDiscovery; timeMs=%d, lastTick=%d\n", timeMs, lastTick);

  json j;
  j["protocol"] = "discovery";
  j["command"] = "participant";

  json payload;
  payload["component"] = "c-base/c_nancy";
  payload["label"] = "CNC mill status";
  payload["icon"] = "scissors";
  payload["inports"] = json::array();

  json outports = json::array();
  json onlinePort;
  onlinePort["id"] = "online";
  onlinePort["type"] = "boolean";
  onlinePort["queue"] = baseTopic_ + "online";

  json millingPort;
  millingPort["id"] = "milling";
  millingPort["type"] = "boolean";
  millingPort["queue"] = baseTopic_ + "milling";

  json positionPort;
  positionPort["id"] = "position";
  positionPort["type"] = "object";
  positionPort["queue"] = baseTopic_ + "position";

  json worktimePort;
  worktimePort["id"] = "worktime";
  worktimePort["type"] = "String";
  worktimePort["queue"] = baseTopic_ + "worktime";

  outports.push_back(onlinePort);
  outports.push_back(millingPort);
  outports.push_back(positionPort);
  outports.push_back(worktimePort);

  payload["outports"] = outports;

  j["role"] = "c_nancy";
  j["id"] = "c_nancy";
  j["payload"] = payload;

  printf("payload object:\n---\n%s\n---\n", j.dump(4).c_str());

  mqttPublish("fbp", j);

  lastTick = timeMs;
}

bool MsgFlo::isMilling() {
  return UC.pGetLed(UcncLed::Cyclestart);
}

void MsgFlo::handleMillingState(long timeMs) {
  bool m = isMilling();
  dbg("Is milling: %d\n", m ? 1 : 0);

  if (isMilling_ != m) {
    isMilling_ = m;

    json j = m;
    mqttPublish("milling", j);
  }
}

void MsgFlo::handlePositionState(long timeMs) {
  bool b = UC.pIsMoving();

  dbg("Is moving:  %d\n", b ? 1 : 0);

  if (!b)
    return;

  json j;
  j["X"] = UC.pGetFieldDouble(true, UcncField::XposDRO) / 10000.0;
  j["Y"] = UC.pGetFieldDouble(true, UcncField::YposDRO) / 10000.0;
  j["Z"] = UC.pGetFieldDouble(true, UcncField::ZposDRO) / 10000.0;
  j["A"] = UC.pGetFieldDouble(true, UcncField::AposDRO) / 10000.0;
  j["B"] = UC.pGetFieldDouble(true, UcncField::BposDRO) / 10000.0;
  j["C"] = UC.pGetFieldDouble(true, UcncField::CposDRO) / 10000.0;

  mqttPublish("position", j);
}

void MsgFlo::handleWorkTime(long timeMs) {
  long lastTick = 0;

  // Only update every 250 ms:
  if (lastTick != 0 && timeMs - lastTick < 250)
    return;

  if (!isMilling())
    return;

  char pField[256];
  UC.pGetField(pField, sizeof(pField), true, UcncField::Worktimer);

  json worktime;
  worktime["worktime"] = pField;

  string worktimeStr = worktime.dump();
  pPaho_->publish(baseTopic_ + "worktime", worktimeStr.c_str(), worktimeStr.length(), 1, false);

  lastTick = timeMs;
}

void MsgFlo::onShutdown() {
  trace();

  json j = false;
  mqttPublish("online", j, MsgRetain::Retain);
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
