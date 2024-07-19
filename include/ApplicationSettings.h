#include <Arduino.h>
#include <Preferences.h>

#define POWER_THRESHOLD "powerThreshold"
#define PAGE_SIZE "pageSize"

class ApplicationSettings {
    public:
    ApplicationSettings();

    String getJSON();

    static ApplicationSettings *instance();

    int getPowerThreshold();
    void setPowerThreshold(int power);

    int getPageSize();
    void setPageSize(int size);

    private:
    static ApplicationSettings *instance_;

    Preferences settings_;
};