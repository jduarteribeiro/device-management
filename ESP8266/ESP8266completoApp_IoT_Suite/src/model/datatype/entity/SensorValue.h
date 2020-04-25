// SensorValue

#ifndef __ENTITY_SENSORVALUE_H__
#define __ENTITY_SENSORVALUE_H__

#include <WString.h>

namespace org_eclipse_vorto_types {
    class SensorValue
    {
        public:
            SensorValue();
            
            void setcurrentMeasured(float value);
            float getcurrentMeasured();
            void setminMeasured(float value);
            float getminMeasured();
            void setmaxMeasured(float value);
            float getmaxMeasured();
            
            String serialize();
        private:
            float currentMeasured;
            float minMeasured;
            float maxMeasured;
    };
}
#endif // __ENTITY_SENSORVALUE_H__
