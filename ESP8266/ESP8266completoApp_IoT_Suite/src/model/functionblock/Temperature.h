// Temperature

#ifndef __MODEL_TEMPERATURE_H__
#define __MODEL_TEMPERATURE_H__

#include <WString.h>


namespace org_eclipse_vorto_tutorial {
    class Temperature
    {
        public:
            Temperature();
            
            void setvalue(float value);
            float getvalue();
            void setunit(String value);
            String getunit();

            String serialize(String ditto_namespace, String hono_deviceId, String fbName);
        private:
            float value;
            String unit;
    };
}

#endif // __MODEL_TEMPERATURE_H__
