// Temperature

#include "Temperature.h"

using namespace org_eclipse_vorto_tutorial;

Temperature::Temperature(){}

void Temperature::setvalue(float _value) {
	value = _value;
}

float Temperature::getvalue() {
	return value;
}
void Temperature::setunit(String _unit) {
	unit = _unit;
}

String Temperature::getunit() {
	return unit;
}


String Temperature::serialize(String ditto_topic, String hono_deviceId, String fbName) {
    String result = "{\"topic\":\""+ ditto_topic +"/things/twin/commands/modify\",";
    result += "\"headers\":{\"response-required\": false},";
    result += "\"path\":\"/features/" + fbName + "/properties\",\"value\": {";
    //Status Properties
    result += "\"status\": {";
    result += "\"value\" : " + String(value) + ",";
    result += "\"unit\" : \"" + String(unit) + "\" ";
    result += "}";


    result += "} }";

    return result;
}
