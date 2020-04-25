// ESP8266completo

#ifndef __INFOMODEL_ESP8266COMPLETO_H__
#define __INFOMODEL_ESP8266COMPLETO_H__

#include <WString.h>

#include "../functionblock/Temperature.h"
#include "../functionblock/Humidity.h"
#include "../functionblock/Illuminance.h"
#include "../functionblock/LocalControlState.h"

namespace vorto_private_joaopedro_bastos {
    class ESP8266completo
    {
       public:
            ESP8266completo();

            org_eclipse_vorto_tutorial::Temperature temperature;
            com_bosch_iot_suite_example_octopussuiteedition::Humidity humidity;
            org_eclipse_vorto::Illuminance illuminance;
            com_bshg_common::LocalControlState localcontrolstate;

            String serialize();
        private:
    };
}

#endif // __INFOMODEL_ESP8266COMPLETO_H__
