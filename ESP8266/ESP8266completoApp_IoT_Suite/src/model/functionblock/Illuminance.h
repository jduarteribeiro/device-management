// Illuminance

#ifndef __MODEL_ILLUMINANCE_H__
#define __MODEL_ILLUMINANCE_H__

#include <WString.h>

#include "../datatype/entity/SensorValue.h"

namespace org_eclipse_vorto {
    class Illuminance
    {
        public:
            Illuminance();
            
            void setvalue(org_eclipse_vorto_types::SensorValue value);
            org_eclipse_vorto_types::SensorValue getvalue();

            String serialize(String ditto_namespace, String hono_deviceId, String fbName);
        private:
            org_eclipse_vorto_types::SensorValue value;
    };
}

#endif // __MODEL_ILLUMINANCE_H__
