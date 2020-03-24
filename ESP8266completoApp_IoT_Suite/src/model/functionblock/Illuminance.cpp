// Illuminance

#include "Illuminance.h"

using namespace org_eclipse_vorto;

Illuminance::Illuminance(){}

void Illuminance::setvalue(org_eclipse_vorto_types::SensorValue _value) {
	value = _value;
}

org_eclipse_vorto_types::SensorValue Illuminance::getvalue() {
	return value;
}


String Illuminance::serialize(String ditto_topic, String hono_deviceId, String fbName) {
    String result = "{\"topic\":\""+ ditto_topic +"/things/twin/commands/modify\",";
    result += "\"headers\":{\"response-required\": false},";
    result += "\"path\":\"/features/" + fbName + "/properties\",\"value\": {";
    //Status Properties
    result += "\"status\": {";
    result += "\"value\" : " + value.serialize();
    result += "}";


    result += "} }";

    return result;
}
