# // Generated by Vorto from org.eclipse.vorto.Temperature

class Temperature(object):
    def __init__(self):
        self.value = 0.0

    ### Status property value
    @property
    def value(self):
        return self.__value[0]
    
    @value.setter
    def value(self, value):
        self.__value = (value, True)
    
    def serializeStatus(self, serializer):
        serializer.first_prop = True
        if self.__value[1]:
               serializer.serialize_property("value", self.__value[0])
               self.__value = (self.__value[0], False)
    def serializeConfiguration(self, serializer):
        pass
