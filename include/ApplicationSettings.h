#include <Arduino.h>
#include <Preferences.h>

#define POWER_THRESHOLD "powerThreshold"
#define PAGE_SIZE "pageSize"
#define HOT_SENSOR_PRESENCE "hotSensor"
#define COLD_SENSOR_PRESENCE "coldSensor"
#define VOLTAGE_SENSOR_PRESENCE "voltageSensor"

class ApplicationSettings {
    public:
    ApplicationSettings();

    String getJSON();
    bool parseJSON(char *jsonTxt);

    static ApplicationSettings *instance();

    int getPowerThreshold();
    void setPowerThreshold(int power);

    int getPageSize();
    void setPageSize(int size);

    bool getHotSensorPresence();
    void setHotSensorPresence(bool present);

    bool getColdSensorPresence();
    void setColdSensorPresence(bool present);

    bool getVoltageSensorPresence();
    void setVoltageSensorPresence(bool present);

    private:
    static ApplicationSettings *instance_;

    bool coldSensorPresence_;
    bool hotTSensorPresence_;
    bool voltageSensorPresence_;

    char* searchKey(const char* jsonTxt, const char* key);

    Preferences settings_;
};