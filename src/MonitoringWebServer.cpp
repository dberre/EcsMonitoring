#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

#include "MonitoringWebServer.h"
#include "utils.h"
#include "Queues.h"
#include "DataPoint.h"

MonitoringWebServer::MonitoringWebServer() {
    server = new AsyncWebServer(80);

    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /");
        request->send(SPIFFS, "/index.html", "text/html");
        watchdogTimer->restart();
    });

    server->on("/immediateMonitoring", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /immediateMonitoring");
        request->send(SPIFFS, "/immediateMonitoring.html", "text/html");
        watchdogTimer->restart();
    });
    
    server->on("/historizedMonitoring", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /historizedMonitoring");
        request->send(SPIFFS, "/historizedMonitoring.html", "text/html");
        watchdogTimer->restart();
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
        acquisitionTimer->suspend();
        request->send(SPIFFS, "/EcsMonitoring.dat", "application/octet-stream");
        acquisitionTimer->resume();
        watchdogTimer->restart();
    });

    server->on("/history", HTTP_GET, [](AsyncWebServerRequest *request) {      
        int count = 1;
        if (request->hasParam("count")) {
            count = request->getParam("count")->value().toInt();
        }

        int offset = 0;
        if (request->hasParam("offset")) {
            offset = request->getParam("offset")->value().toInt();
        }

        Serial.printf("server: /history %d %d\n", count, offset);

        RequestQueueMsg req = GetHistoryRequest(count, offset);
        
        if (xQueueSend(requestQueue, &req, 0) == pdTRUE) {
            ResponseQueueMsg response;
            uint32_t t0 = millis();

            while (true) {
                if ((millis() - t0) > 1000) {
                    // time out, no response received
                    request->send(500);
                    break;
                } else if (xQueueReceive(responseQueue, &response, 100 / portTICK_PERIOD_MS) == pdTRUE) {
                    if (response.count > 0) {
                        String result = String();
                        char date_string[21];
                        char csvStr[100];
                        for (int i = 0; i < response.count; i++) {
                            DataPoint newPoint = response.points[i];
                            strftime(date_string, 20, "%d/%m/%y %T", localtime(&(newPoint.timestamp)));
                
                            sprintf(csvStr, "%s,%.01f,%.01f,%s\n",
                                date_string,
                                (float)newPoint.coldTemperature / 10.0f,
                                (float)newPoint.hotTemperature / 10.0f,
                                newPoint.heating ? "Oui" : "Non");

                            result.concat(csvStr);
                        }
                        request->send(200, "text/plain", result);
                    } else {
                        // response is empty
                        request->send(500);
                    }
                    delete(response.points);
                    break;
                }
            }
        } else {
            // the queue is full, can't send the request
            request->send(500);
        }

        watchdogTimer->restart();
    });

    server->on("/clearHistory", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /clearHistory");
        RequestQueueMsg req = ClearHistoryRequest;
        if (xQueueSend(requestQueue, &req, 0) == pdTRUE) {
            request->send(200);
        } else {
            request->send(500);
        }
    });

    server->on("/getInstantValues", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /getInstantValues");
        watchdogTimer->restart();

        RequestQueueMsg req = GetMeasurementRequest;
        if (xQueueSend(requestQueue, &req, 0) == pdTRUE) {
            ResponseQueueMsg response;
            uint32_t t0 = millis();
            bool received = false;
            while (!received) {
                if ((millis() - t0) > 1500) {
                    // time out, no response received
                    request->send(500);
                    break;
                } else if (xQueueReceive(responseQueue, &response, 100 / portTICK_PERIOD_MS) == pdTRUE) {
                    if (response.count == 1) {  
                        char date_string[21];
                        strftime(date_string, 20, "%d/%m/%y %T", localtime(&(response.points[0].timestamp)));
                        
                        char jsonStr[100];
                        sprintf(jsonStr, "{ \"time\":\"%s\",\"cold\":\"%.01f\",\"hot\":\"%.01f\",\"state\":\"%s\" }",
                            date_string,
                            (float)response.points[0].coldTemperature / 10.0f,
                            (float)response.points[0].hotTemperature / 10.0f,
                            response.points[0].heating ? "Oui" : "Non");
                        request->send(200, "application/json", String(jsonStr));
                        received = true;
                    }
                    delete(response.points);
                }
            }
        } else {
            // the queue is full, can't send the request
            request->send(500);
        }
    });

    server->on("/setTime", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.printf("server: /setTime\n");

        if (request->hasParam("timeEpoch")) {
            String param = request->getParam("timeEpoch")->value();
            timeval tmval;
            tmval.tv_sec = param.toInt();
            tmval.tv_usec = 0;
            settimeofday(&tmval, NULL);
            
            char date_string[21];
            time_t now = time(NULL);
            strftime(date_string, 20, "%d/%m/%y %T", localtime(&now));
            Serial.printf("received:%d set:%s\n", param.toInt(), date_string);

            request->send(200, "application/json", String("{\"timeEpoch\":") + String(now) + String("}"));
        } else {
            request->send(500);
        }
        watchdogTimer->restart();
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