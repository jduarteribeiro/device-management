// Humidity

#include "Humidity.h"

using namespace com_bosch_iot_suite_example_octopussuiteedition;

Humidity::Humidity(){}

void Humidity::setsensorValue(float _sensorValue) {
	sensorValue = _sensorValue;
}

float Humidity::getsensorValue() {
	return sensorValue;
}
void Humidity::setminMeasuredValue(float _minMeasuredValue) {
	minMeasuredValue = _minMeasuredValue;
}

float Humidity::getminMeasuredValue() {
	return minMeasuredValue;
}
void Humidity::setmaxMeasuredValue(float _maxMeasuredValue) {
	maxMeasuredValue = _maxMeasuredValue;
}

float Humidity::getmaxMeasuredValue() {
	return maxMeasuredValue;
}
void Humidity::setminRangeValue(float _minRangeValue) {
	minRangeValue = _minRangeValue;
}

float Humidity::getminRangeValue() {
	return minRangeValue;
}
void Humidity::setmaxRangeValue(float _maxRangeValue) {
	maxRangeValue = _maxRangeValue;
}

float Humidity::getmaxRangeValue() {
	return maxRangeValue;
}
void Humidity::setsensorUnits(String _sensorUnits) {
	sensorUnits = _sensorUnits;
}

String Humidity::getsensorUnits() {
	return sensorUnits;
}


String Humidity::serialize(String ditto_topic, String hono_deviceId, String fbName) {
    String result = "{\"topic\":\""+ ditto_topic +"/things/twin/commands/modify\",";
    result += "\"headers\":{\"response-required\": false},";
    result += "\"path\":\"/features/" + fbName + "/properties\",\"value\": {";
    //Status Properties
    result += "\"status\": {";
    result += "\"sensorValue\" : " + String(sensorValue) + ",";
    result += "\"minMeasuredValue\" : " + String(minMeasuredValue) + ",";
    result += "\"maxMeasuredValue\" : " + String(maxMeasuredValue) + ",";
    result += "\"minRangeValue\" : " + String(minRangeValue) + ",";
    result += "\"maxRangeValue\" : " + String(maxRangeValue) + ",";
    result += "\"sensorUnits\" : \"" + String(sensorUnits) + "\" ";
    result += "}";


    result += "} }";

    return result;
}
