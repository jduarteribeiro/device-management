// Humidity

#ifndef __MODEL_HUMIDITY_H__
#define __MODEL_HUMIDITY_H__

#include <WString.h>


namespace com_bosch_iot_suite_example_octopussuiteedition {
    class Humidity
    {
        public:
            Humidity();
            
            void setsensorValue(float value);
            float getsensorValue();
            void setminMeasuredValue(float value);
            float getminMeasuredValue();
            void setmaxMeasuredValue(float value);
            float getmaxMeasuredValue();
            void setminRangeValue(float value);
            float getminRangeValue();
            void setmaxRangeValue(float value);
            float getmaxRangeValue();
            void setsensorUnits(String value);
            String getsensorUnits();

            String serialize(String ditto_namespace, String hono_deviceId, String fbName);
        private:
            float sensorValue;
            float minMeasuredValue;
            float maxMeasuredValue;
            float minRangeValue;
            float maxRangeValue;
            String sensorUnits;
    };
}

#endif // __MODEL_HUMIDITY_H__
