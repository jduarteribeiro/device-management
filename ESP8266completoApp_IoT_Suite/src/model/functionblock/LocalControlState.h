// LocalControlState

#ifndef __MODEL_LOCALCONTROLSTATE_H__
#define __MODEL_LOCALCONTROLSTATE_H__

#include <WString.h>


namespace com_bshg_common {
    class LocalControlState
    {
        public:
            LocalControlState();
            
            void setcurrentState(bool value);
            bool getcurrentState();

            String serialize(String ditto_namespace, String hono_deviceId, String fbName);
        private:
            bool currentState;
    };
}

#endif // __MODEL_LOCALCONTROLSTATE_H__
