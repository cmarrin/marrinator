#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiManager.h>

#ifndef NDEBUG
#include <GDBStub.h>
#endif

#include "Application.h"
#include <chrono>

bool ready = false;
bool state = false;

m8r::Application* _application = nullptr;

void setup()
{
    delay(500);
    Serial.begin(115200);

#ifndef NDEBUG
    gdbstub_init();
#endif    

    _application = new m8r::Application(23);

    // Test filesystem
    Serial.println("\n\nTesting Filesystem:");
    m8r::String toPath("/foo");
    m8r::Mad<m8r::File> toFile(m8r::system()->fileSystem()->open(toPath.c_str(), m8r::FS::FileOpenMode::Write));

    if (!toFile->valid()) {
        Serial.printf("Error: unable to open '%s' for write - ", toPath.c_str());
        m8r::Error::showError(toFile->error());
        Serial.printf("\n");
    } else {
        toFile->write("Hello World", 11);
        if (!toFile->valid()) {
            Serial.printf("Error writing '%s' - ", toPath.c_str());
            m8r::Error::showError(toFile->error());
            Serial.printf("\n");
        } else {
            Serial.printf("Successfully wrote '%s'\n", toPath.c_str());
        }
        toFile->close();
    }

    toFile = m8r::Mad<m8r::File>(m8r::system()->fileSystem()->open(toPath.c_str(), m8r::FS::FileOpenMode::Read));
    if (!toFile->valid()) {
        Serial.printf("Error: unable to open '%s' for read - ", toPath.c_str());
        m8r::Error::showError(toFile->error());
        Serial.printf("\n");
    } else {
        char buf[12];
        int32_t result = toFile->read(buf, 11);
        if (!toFile->valid()) {
            Serial.printf("Error reading '%s' - ", toPath.c_str());
            m8r::Error::showError(toFile->error());
            Serial.printf("\n");
        } else if (result != 11) {
            Serial.printf("Wrong number of bytes read from '%s', expected 11, got %d\n", toPath.c_str(), result);
        } else {
            buf[11] = '\0';
            Serial.printf("Successfully read '%s' - '%s'\n", toPath.c_str(), buf);
        }
        toFile->close();
    }
}

void loop()
{
    _application->runOneIteration();

    // if (ready) {
    //     ready = false;
    //     state = !state;
    //     digitalWrite(LED_BUILTIN, state ? HIGH : LOW);

    //     if (state) {
    //         std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    //         uint64_t t = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
    //         Serial.printf("******** now = %d\n", (int) t);
    //     }
    // }
}