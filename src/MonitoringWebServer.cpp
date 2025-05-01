#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <LittleFS.h>

#include "MonitoringWebServer.h"
#include "utils.h"
#include "Queues.h"
#include "DataPoint.h"
#include "ApplicationSettings.h"

MonitoringWebServer::MonitoringWebServer() {
    server = new AsyncWebServer(80);

    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /");
        request->send(LittleFS, "/index.html", "text/html");
        watchdogTimer->restart();
    });

    server->on("/immediateMonitoring", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /immediateMonitoring");
        request->send(LittleFS, "/immediateMonitoring.html", "text/html");
        watchdogTimer->restart();
    });
    
    server->on("/voltageMonitoring", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /voltageMonitoring");
        request->send(LittleFS, "/voltageMonitoring.html", "text/html");
        watchdogTimer->restart();
    });
    
    server->on("/historizedMonitoring", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /historizedMonitoring");
        request->send(LittleFS, "/historizedMonitoring.html", "text/html");
        watchdogTimer->restart();
    });
 
    server->on("/downloadHistory", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /downloadHistory");
        request->send(LittleFS, "/downloadHistory.html", "text/html");
        watchdogTimer->restart();
    });

    server->on("/preferences", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /preferences");
        request->send(LittleFS, "/preferences.html", "text/html");
        watchdogTimer->restart();
    });
    
    server->on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/script.js", "text/javascript");
    });
    
    server->on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/w3.css", "text/css");
    });

    server->on("/jquery-3.6.3.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /jquery-3.6.3.min.js");
        request->send(LittleFS, "/jquery-3.6.3.min.js", "text/javascript");
    });

    server->on("/gotoLightSleep", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /gotoLightSleep");
        RequestQueueMsg req = GotoLightSleepRequest;
        if (xQueueSend(requestQueue, &req, 0) == pdTRUE) {
            request->send(200);
        } else {
            request->send(500);
        }
    });

    server->on("/gotoDeepSleep", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /gotoDeepSleep");
        request->send(200);
        // the answer is sent before as deep sleep stops the server
        gotoDeepSleep();
    });

    server->on("/downloadHistoryRaw", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /downloadHistoryRaw");
        acquisitionTimer->suspend();
        request->send(LittleFS, "/EcsMonitoring.dat", "application/octet-stream");
        acquisitionTimer->resume();
        watchdogTimer->restart();
    });

    server->on("/history", HTTP_GET, [](AsyncWebServerRequest *request) {      
        int count = 10;
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
                    if (response.msgType == ResponseQueueMsg::MsgType::series) {
                        String result = String();
                        char date_string[21];
                        char csvStr[100];
                        for (int i = 0; i < response.data.series.count; i++) {
                            DataPoint newPoint = response.data.series.points[i];
                            strftime(date_string, 20, "%d/%m/%y %T", localtime(&(newPoint.timestamp)));
                
                            sprintf(csvStr, "%s,%.01f,%.01f,%.01f\n",
                                date_string,
                                (float)newPoint.coldTemperature / 10.0f,
                                (float)newPoint.hotTemperature / 10.0f,
                                newPoint.power);

                            result.concat(csvStr);
                        }
                        request->send(200, "text/plain", result);
                        delete(response.data.series.points);
                        break;
                    } 
                }
            }
        } else {
            // the queue is full, can't send the request
            request->send(500);
        }

        watchdogTimer->restart();
    });

    server->on("/historyDepth", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /historyDepth");
        watchdogTimer->restart();

        RequestQueueMsg req = GetHistoryDepth;
        if (xQueueSend(requestQueue, &req, 0) == pdTRUE) {
            ResponseQueueMsg response;
            uint32_t t0 = millis();
            bool received = false;
            while (!received) {
                if ((millis() - t0) > 1000) {
                    // time out, no response received
                    request->send(500);
                    break;
                } else if (xQueueReceive(responseQueue, &response, 100 / portTICK_PERIOD_MS) == pdTRUE) {
                    if (response.msgType == ResponseQueueMsg::MsgType::series) {
                        char jsonStr[40];
                        sprintf(jsonStr, "{\"historyDepth\":%d}", response.data.series.count);
                        request->send(200, "application/json", String(jsonStr));
                        received = true;
                    }
                }
            }
        } else {
            // the queue is full, can't send the request
            request->send(500);
        }
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
                    if (response.msgType == ResponseQueueMsg::MsgType::series) {
                        if (response.data.series.count == 1) {  
                            char date_string[21];
                            strftime(date_string, 20, "%d/%m/%y %T", localtime(&(response.data.series.points[0].timestamp)));
                            
                            char jsonStr[100];
                            sprintf(jsonStr, "{ \"time\":\"%s\",\"cold\":%.01f,\"hot\":%.01f,\"power\":%.01f }",
                                date_string,
                                (float)response.data.series.points[0].coldTemperature / 10.0f,
                                (float)response.data.series.points[0].hotTemperature / 10.0f,
                                response.data.series.points[0].power);
                            request->send(200, "application/json", String(jsonStr));
                            received = true;
                        }
                        delete(response.data.series.points);
                    }
                }
            }
        } else {
            // the queue is full, can't send the request
            request->send(500);
        }
    });

    server->on("/getVoltage", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /getVoltage");
        watchdogTimer->restart();

        RequestQueueMsg req = GetVoltageRequest;
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
                    if (response.msgType == ResponseQueueMsg::MsgType::voltage) {
                        char jsonStr[40];
                        sprintf(jsonStr, "{\"voltage\":%.06f}", response.data.voltage);
                        request->send(200, "application/json", String(jsonStr));
                        received = true;
                    }
                }
            }
        } else {
            // the queue is full, can't send the request
            request->send(500);
        }
    });

    server->on("/getBatteryVoltage",HTTP_GET, [](AsyncWebServerRequest *request) {
        // example: curl http://192.168.4.22/getBatteryVoltage
        Serial.println("server: /getBatteryVoltage");
        watchdogTimer->restart();

        RequestQueueMsg req = GetBatteryVoltageRequest;
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
                    if (response.msgType == ResponseQueueMsg::MsgType::voltage) {
                        char jsonStr[40];
                        sprintf(jsonStr, "{\"voltage\":%.06f}", response.data.voltage);
                        request->send(200, "application/json", String(jsonStr));
                        received = true;
                    }
                }
            }
        } else {
            // the queue is full, can't send the request
            request->send(500);
        }
    });
    
    server->on("/setTime", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /setTime");

        if (request->hasParam("timeEpoch")) {
            String param = request->getParam("timeEpoch")->value();
            timeval tmval;
            tmval.tv_sec = param.toInt();
            tmval.tv_usec = 0;
            settimeofday(&tmval, NULL);
            
            char date_string[21];
            time_t now = time(NULL);
            strftime(date_string, 20, "%d/%m/%y %T", localtime(&now)); // FIXME remove
            Serial.printf("received:%d set:%s\n", param.toInt(), date_string);

            request->send(200, "application/json", String("{\"timeEpoch\":") + String(now) + String("}"));
        } else {
            request->send(400);
        }
        watchdogTimer->restart();
    });

    server->on("/getPreferences", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /getPreferences");
        watchdogTimer->restart();
        String response = ApplicationSettings::instance()->getJSON();
        request->send(200, "application/json", response);
    });

    server->on("/flush", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("server: /flush");
        watchdogTimer->restart();

        ResponseQueueMsg response;
        while (xQueueReceive(responseQueue, &response, 1 / portTICK_PERIOD_MS) == pdTRUE) {
            Serial.printf("/fush %d\n", response.msgType);
        }
        request->send(200);
    });

    server->onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        Serial.println("server: onRequestBody");
        // example for testing:  curl -i -d '{"voltageSensorPresence":1}' -H "Content-Type: application/json" -X POST http://192.168.4.22

        char *jsonTxt = strndup((const char*)data, len);
        if (ApplicationSettings::instance()->parseJSON(jsonTxt)) {
            free(jsonTxt);
            acquisitionTimer->stop();
            acquisitionTimer->setDuration(ApplicationSettings::instance()->getSamplingPeriod());
            acquisitionTimer->start();
            request->send(200);
        } else {
            request->send(400);
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