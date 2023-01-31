#ifndef _TEMPERATURE_SENSOR_H
#define _PERSISTENCE_H

bool setupTemperatureSensors(void);
void requestColdTemperature();
void requestHotTemperature();
float getColdTemperature();
float getHotTemperature();

#endif /* _TEMPERATURE_SENSOR_H */