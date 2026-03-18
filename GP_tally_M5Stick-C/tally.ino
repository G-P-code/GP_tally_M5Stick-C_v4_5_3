// ============================================================================
// tally.ino — TallyArbiter Socket.IO communication and tally data processing
//
// Socket.IO v2 implemented directly over WebSocketsClient (links2004/WebSockets).
// JSON parsing uses ArduinoJson v7.
//
// Socket.IO v2 wire protocol:
//   Engine.IO packet types (first char):
//     '0' — open (server sends session info)
//     '2' — ping  (server → client, we reply '3')
//     '3' — pong
//     '4' — message (contains Socket.IO packet)
//   Socket.IO packet types (second char, inside '4...'):
//     '0' — connect
//     '2' — event  → payload is JSON array ["eventName", ...data]
// ============================================================================

// ============================================================================
// Socket.IO connection
// ============================================================================

void connectToServer() {
  logger("Connecting to Tally Arbiter: " + String(tallyarbiter_host) + ":" + String(tallyarbiter_port), "info");
  socket.onEvent(wsEvent);
  socket.begin(tallyarbiter_host, atol(tallyarbiter_port), "/socket.io/?EIO=4&transport=websocket");
}

// Emit a Socket.IO event with optional JSON payload string
void ws_emit(String event, const char *payload) {
  String msg = "42[\"" + event + "\"";
  if (payload) {
    msg += ",";
    msg += payload;
  }
  msg += "]";
  socket.sendTXT(msg);
}

// ============================================================================
// WebSocket event handler — Engine.IO + Socket.IO dispatch
// ============================================================================

void wsEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {

    case WStype_DISCONNECTED:
      onSocketDisconnect();
      break;

    case WStype_CONNECTED:
      // WebSocket connected — send Socket.IO connect packet
      socket.sendTXT("40");
      break;

    case WStype_TEXT: {
      String msg = (char *)payload;
      if (msg.length() == 0) break;

      char eioType = msg[0];

      if (eioType == '2') {
        // Engine.IO ping — reply with pong
        socket.sendTXT("3");
        break;
      }

      if (eioType == '4' && msg.length() > 1) {
        char sioType = msg[1];

        if (sioType == '0') {
          // Socket.IO connect ACK
          onSocketConnect();
          break;
        }

        if (sioType == '2') {
          // Socket.IO event
          onSocketEvent(msg.substring(2));
          break;
        }
      }
      break;
    }

    case WStype_ERROR:
      logger("WebSocket error", "error");
      break;


    default:
      break;
  }
}

// ============================================================================
// Socket.IO application events
// ============================================================================

void onSocketConnect() {
  logger("Connected to Tally Arbiter server.", "info");
  logger("DeviceId: " + DeviceId, "info-quiet");

  char deviceObj[256];
  snprintf(deviceObj, sizeof(deviceObj),
    "{\"deviceId\":\"%s\",\"listenerType\":\"%s\","
    "\"canBeReassigned\":true,\"canBeFlashed\":true,\"supportsChat\":true}",
    DeviceId.c_str(), listenerDeviceName.c_str());
  ws_emit("listenerclient_connect", deviceObj);

  taConnected    = true;
  everConnected  = true;
  reconnectDelay = RECONNECT_BASE_DELAY;

  if (currentScreen == 1 && !userRequestedSettings) {
    switchToTally();
  } else if (currentScreen == 1) {
    paintSettingsScreen();
  }
}

void onSocketDisconnect() {
  taConnected     = false;
  busOptionsReady = false;
  actualType      = "";
  actualColor     = "";
  actualPriority  = 0;
  prevType        = "__invalidated__";
  prevLedType     = "__invalidated__";
#if TALLY_EXTRA_OUTPUT
  externalLed();
  prevLedType = actualType;
#endif

  if (!everConnected) {
    paintSettingsScreen();
  } else {
    if (currentScreen == 0) {
      switchToSettings();
    } else {
      paintSettingsScreen();
    }
  }
}

void onSocketEvent(String raw) {
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, raw);
  if (err) {
    logger("JSON parse error: " + String(err.c_str()), "error");
    return;
  }

  if (!doc.is<JsonArray>() || doc.as<JsonArray>().size() < 1) return;

  String eventType = doc[0].as<String>();
  logger("Got event '" + eventType + "'", "info-quiet");

  if (eventType == "bus_options") {
    jBusOptions.clear();
    jBusOptions.set(doc[1]);
    busOptionsReady = true;
    if (!jDeviceStates.isNull()) processTallyData();

  } else if (eventType == "devices") {
    jDevices.clear();
    jDevices.set(doc[1]);
    SetDeviceName();

  } else if (eventType == "device_states") {
    jDeviceStates.clear();
    jDeviceStates.set(doc[1]);
    if (busOptionsReady) processTallyData();

  } else if (eventType == "deviceId") {
    DeviceId = doc[1].as<String>();
    SetDeviceName();
    if (!userRequestedSettings) switchToTally();

  } else if (eventType == "reassign") {
    socket_Reassign(doc[1].as<String>(), doc[2].as<String>());

  } else if (eventType == "flash") {
    socket_Flash();

  } else if (eventType == "messaging") {
    String msgType = doc[1].as<String>();
    String msg     = doc[3].as<String>();
    LastMessage = msgType + ": " + msg;
    evaluateMode();
  }
}

// ============================================================================
// Flash animation trigger
// ============================================================================

void socket_Flash() {
  flashActive = true;
  flashStep   = 0;
  flashLastMs = 0;
}

// ============================================================================
// Reassign
// ============================================================================

void socket_Reassign(String oldDeviceId, String newDeviceId) {
  logger("Reassign: " + oldDeviceId + " -> " + newDeviceId, "info");

  char charReassignObj[256];
  snprintf(charReassignObj, sizeof(charReassignObj),
    "{\"oldDeviceId\":\"%s\",\"newDeviceId\":\"%s\"}",
    oldDeviceId.c_str(), newDeviceId.c_str());
  ws_emit("listener_reassign_object", charReassignObj);
  ws_emit("devices", nullptr);

  DeviceId = newDeviceId;
  preferences.begin(PREF_NAMESPACE, false);
  preferences.putString(PREF_DEVICE_ID, newDeviceId);
  preferences.end();

  // Clear tally state immediately — new device state unknown until device_states arrives
  actualType     = "";
  actualColor    = "";
  actualPriority = 0;
  prevType       = "__invalidated__";
  prevLedType    = "__invalidated__";
#if TALLY_EXTRA_OUTPUT
  externalLed();
  prevLedType = actualType;
#endif

  SetDeviceName();

  reassignActive = true;
  reassignStep   = 0;
  reassignLastMs = 0;
}

// ============================================================================
// Tally data processing
// ============================================================================

void processTallyData() {
  bool   typeChanged     = false;
  int    highestPriority = -1;
  String highestType     = "";
  String highestColor    = "";
  int    highestPrioVal  = 0;

  JsonArray states = jDeviceStates.as<JsonArray>();
  for (JsonObject state : states) {
    if (state["deviceId"].as<String>() != DeviceId) continue;

    JsonArray sources = state["sources"].as<JsonArray>();
    if (sources.size() == 0) continue;

    typeChanged = true;
    String busId = state["busId"].as<String>();

    String busType, busColor;
    int    busPrio = 0;
    getBusInfo(busId, busType, busColor, busPrio);

    if (busPrio > highestPriority) {
      highestPriority = busPrio;
      highestType     = busType;
      highestColor    = busColor;
      highestPrioVal  = busPrio;
    }
  }

  if (typeChanged) {
    actualType     = highestType;
    actualColor    = highestColor;
    actualPriority = highestPrioVal;
  } else {
    actualType     = "";
    actualColor    = "";
    actualPriority = 0;
  }

  if (typeChanged && currentScreen == 1 && !userRequestedSettings) {
    switchToTally();
    return;
  }

  evaluateMode();
}

void getBusInfo(const String &busId, String &outType, String &outColor, int &outPriority) {
  outType     = "";
  outColor    = "";
  outPriority = 0;

  JsonArray buses = jBusOptions.as<JsonArray>();
  for (JsonObject bus : buses) {
    if (bus["id"].as<String>() == busId) {
      outType     = bus["type"].as<String>();
      outColor    = bus["color"].as<String>();
      outPriority = bus["priority"].as<int>();
      return;
    }
  }
}

// ============================================================================
// Device name lookup
// ============================================================================

void SetDeviceName() {
  JsonArray devices = jDevices.as<JsonArray>();
  for (JsonObject dev : devices) {
    if (dev["id"].as<String>() == DeviceId) {
      DeviceName = dev["name"].as<String>();
      break;
    }
  }

  preferences.begin(PREF_NAMESPACE, false);
  preferences.putString(PREF_DEVICE_NAME, DeviceName);
  preferences.end();

  logger("DeviceName: " + DeviceName, "info");
  prevType = "__invalidated__";
  evaluateMode();
}
