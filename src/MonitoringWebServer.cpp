#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

#include "MonitoringWebServer.h"
#include "utils.h"
#include "DataPoint.h"

MonitoringWebServer::MonitoringWebServer() {
    server = new AsyncWebServer(80);

    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /");
        request->send(SPIFFS, "/index.html", "text/html");
    });

    server->on("/immediateMonitoring", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /immediateMonitoring");
        request->send(SPIFFS, "/immediateMonitoring.html", "text/html");
    });
    
    server->on("/historizedMonitoring", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /historizedMonitoring");
        request->send(SPIFFS, "/historizedMonitoring.html", "text/html");
    });
    
    server->on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/script.js", "text/javascript");
    });
    
    server->on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/w3.css", "text/css");
    });

    server->on("/jquery-3.6.3.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /jquery-3.6.3.min.js");
        request->send(SPIFFS, "/jquery-3.6.3.min.js", "text/javascript");
    });

    server->on("/gotoSleep", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /gotoSleep");
        gotoSleep();
        request->send(200);
    });

    server->on("/downloadHistory", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /downloadHistory");
        request->send(SPIFFS, "/EcsMonitoring.tsv", "text/plain");
    });

    server->on("/getInstantValues", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /getInstantValues");
        DataPoint newPoint = makeMeasurement();

        char date_string[21];
        strftime(date_string, 20, "%d/%m/%y %T", localtime(&(newPoint.timestamp)));
        
        char jsonStr[100];
        sprintf(jsonStr, "{ \"time\":\"%s\",\"cold\":\"%.01f\",\"hot\":\"%.01f\",\"state\":\"%s\" }",
            date_string,
            (float)newPoint.coldTemperature / 10.0f,
            (float)newPoint.hotTemperature / 10.0f,
             newPoint.heating ? "Oui" : "Non");
        request->send(200, "application/json", String(jsonStr));
    });

    server->on("/setTime", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.printf("server: /setTime\n");
                // int params = request->params();
                // Serial.printf("%d params sent in\n", params);
                // for (int i = 0; i < params; i++)
                // {
                //     AsyncWebParameter *p = request->getParam(i);
                //     if (p->isFile())
                //     {
                //         Serial.printf("_FILE[%s]: %s\n, size: %u", p->name().c_str(), p->value().c_str(), p->size());
                //     }
                //     else if (p->isPost())
                //     {
                //         Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
                //     }
                //     else
                //     {
                //         Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
                //     }
                // }
        if (request->hasParam("timeEpoch")) {
            String param = request->getParam("timeEpoch")->value();
            timeval tmval;
            tmval.tv_sec = param.toInt();
            tmval.tv_usec = 0;
            settimeofday(&tmval, NULL);

            // see https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
            setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/ 3", 1);
            tzset();

            char date_string[21];
            time_t now = time(NULL);
            strftime(date_string, 20, "%d/%m/%y %T", localtime(&now));
            Serial.printf("received:%d set:%s\n", param.toInt(), date_string);

            request->send(204);
        } else {
            request->send(500);
        }
    });
}

bool MonitoringWebServer::start() {
    const IPAddress local_IP(192,168,4,22);
    const IPAddress gateway(192,168,4,9);
    const IPAddress subnet(255,255,255,0);

    bool status = WiFi.softAPConfig(local_IP, gateway, subnet);
    Serial.printf("soft-AP configuration status: %d\n", status ? 1 : 0);

    status = WiFi.softAP(ssid);
    Serial.printf("soft-AP setup status: %d\n", status ? 1 : 0);
 
    Serial.printf("Soft-AP IP address = %s\n", WiFi.softAPIP().toString());
    
    server->begin();

    return true;
}

void MonitoringWebServer::stop() {
    server->end();
}