#include <Arduino.h>
#include <Preferences.h>

#define POWER_THRESHOLD "powerThreshold"
#define TEMPERATURE_THRESHOLD "heatThreshold"
#define PAGE_SIZE "pageSize"
#define HOT_SENSOR_PRESENCE "hotSensorOn"
#define COLD_SENSOR_PRESENCE "coldSensorOn"
#define VOLTAGE_SENSOR_PRESENCE "voltageSensorOn"
#define STORAGE_MODE "storageMode"
#define SAMPLING_PERIOD "samplingPeriod"

// size of concantenated string: 14 + 13 + 8 + 11 +12 + 15 + 11 + 14 = 98

class ApplicationSettings {
    public:
    ApplicationSettings();

    enum StorageMode: int { full = 1, delta };

    String getJSON();
    bool parseJSON(char *jsonTxt);

    static ApplicationSettings *instance();

    int getPowerThreshold();
    void setPowerThreshold(int power);

    int getTemperatureThreshold();
    void setTemperatureThreshold(int temperature);

    int getPageSize();
    void setPageSize(int size);

    bool getHotSensorPresence();
    void setHotSensorPresence(bool present);

    bool getColdSensorPresence();
    void setColdSensorPresence(bool present);

    bool getVoltageSensorPresence();
    void setVoltageSensorPresence(bool present);

    StorageMode getStorageMode();
    void setStorageMode(StorageMode mode);

    int getSamplingPeriod();
    void setSamplingPeriod(int period);


    private:
    static ApplicationSettings *instance_;

    bool coldSensorPresence_;
    bool hotTSensorPresence_;
    bool voltageSensorPresence_;
    StorageMode storageMode_;
    int temperatureThreshold_;
    int samplingPeriod_;

    char* searchKey(const char* jsonTxt, const char* key);

    Preferences settings_;
};