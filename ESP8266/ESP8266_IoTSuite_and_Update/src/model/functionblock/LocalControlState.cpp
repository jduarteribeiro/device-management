// LocalControlState

#include "LocalControlState.h"

using namespace com_bshg_common;

LocalControlState::LocalControlState(){}

void LocalControlState::setcurrentState(bool _currentState) {
	currentState = _currentState;
}

bool LocalControlState::getcurrentState() {
	return currentState;
}


String LocalControlState::serialize(String ditto_topic, String hono_deviceId, String fbName) {
    String result = "{\"topic\":\""+ ditto_topic +"/things/twin/commands/modify\",";
    result += "\"headers\":{\"response-required\": false},";
    result += "\"path\":\"/features/" + fbName + "/properties\",\"value\": {";
    //Status Properties
    result += "\"status\": {";
    result += "\"currentState\" : " + String(currentState == 1 ? "true" : "false") + "";
    result += "}";


    result += "} }";

    return result;
}
