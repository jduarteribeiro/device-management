// SensorValue

#include "SensorValue.h"

using namespace org_eclipse_vorto_types;

SensorValue::SensorValue(){}

void SensorValue::setcurrentMeasured(float _currentMeasured) {
    currentMeasured = _currentMeasured;
}

float SensorValue::getcurrentMeasured() {
    return currentMeasured;
}
void SensorValue::setminMeasured(float _minMeasured) {
    minMeasured = _minMeasured;
}

float SensorValue::getminMeasured() {
    return minMeasured;
}
void SensorValue::setmaxMeasured(float _maxMeasured) {
    maxMeasured = _maxMeasured;
}

float SensorValue::getmaxMeasured() {
    return maxMeasured;
}

String SensorValue::serialize() {
String result = "{";
        result += "\"currentMeasured\": " + String(currentMeasured) + ",";
        result += "\"minMeasured\": " + String(minMeasured) + ",";
        result += "\"maxMeasured\": " + String(maxMeasured);
        result += "}";

    return result;
}
