#ifndef _MONITORING_WEB_SERVER_H
#define _MONITORING_WEB_SERVER_H

#include <ESPAsyncWebServer.h>

class MonitoringWebServer {
    public:
    MonitoringWebServer();
    bool start();
    void stop();

    private:
    const char *ssid = "EcsMonitoring";

    AsyncWebServer *server;
};

#endif /* _MONITORING_WEB_SERVER_H */