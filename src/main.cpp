#include <knx.h>
#include <Wire.h>
#include "hardwareENO.h"
#include "wiring_private.h" // pinPeripheral() function
#include "Enocean.h"
#include "EnoceanGateway.h"
#include <OpenKNX.h>
#include "EnOceanHandle.h"

#ifdef SEEED_XIAO_M0
// XIAO PINS   RX = D3  TX = D2
Uart Serial2(&sercom2, 3, 2, SERCOM_RX_PAD_3, UART_TX_PAD_2);

void SERCOM2_Handler()
{
    Serial2.IrqHandler();
}
#endif

EnOceanDevice device[MAX_NUMBER_OF_DEVICES] = {EnOceanDevice()};

void appSetup();
void appLoop();

void setup()
{
    //Wire.begin();
#ifdef ARDUINO_ARCH_RP2040
    // die defines musst du dann noch setzen
    Serial1.setRX(KNX_UART_RX_PIN);
    Serial1.setTX(KNX_UART_TX_PIN);
    Serial2.setRX(ENO_UART_RX_PIN);
    Serial2.setTX(ENO_UART_TX_PIN);
#endif

    pinMode(PROG_LED_PIN, OUTPUT);
    digitalWrite(PROG_LED_PIN, HIGH);
    delay(6000);
    digitalWrite(PROG_LED_PIN, LOW);
#ifdef KDEBUG_min
    SERIAL_PORT.begin(115200);
    SERIAL_PORT.println("Startup called...");
    ArduinoPlatform::SerialDebug = &SERIAL_PORT;
#endif

#ifdef LED_YELLOW_PIN
    pinMode(LED_YELLOW_PIN, OUTPUT);
    digitalWrite(LED_YELLOW_PIN, HIGH);
#endif

    // knx.readMemory();
    OpenKNX::knxRead(MAIN_OpenKnxId, MAIN_ApplicationNumber, MAIN_ApplicationVersion, 0);

    // pin or GPIO the programming led is connected to. Default is LED_BUILDIN
    knx.ledPin(PROG_LED_PIN);
    // is the led active on HIGH or low? Default is LOW
    knx.ledPinActiveOn(PROG_LED_PIN_ACTIVE_ON);
    // pin or GPIO programming button is connected to. Default is 0
    knx.buttonPin(PROG_BUTTON_PIN);
    // Is the interrup created in RISING or FALLING signal? Default is RISING
    knx.buttonPinInterruptOn(PROG_BUTTON_PIN_INTERRUPT_ON);


    // print values of parameters if device is already configured
    appSetup();


    // start the framework.
    knx.start();


    // start Enocean
    for (int i = 0; i < MAX_NUMBER_OF_DEVICES; i++)
    {
        enOcean.configureDevice(device[i], i);
    }

    Serial2.begin(57600); // Change to Serial wenn original Platine
                          // Assign pins 2 & 3 SERCOM functionality
#ifndef ARDUINO_ARCH_RP2040
    pinPeripheral(2, PIO_SERCOM_ALT);
    pinPeripheral(3, PIO_SERCOM_ALT);
#endif
    enOcean.initSerial(Serial2);
    enOcean.init();

    // Set own BAse-ID for each Channel
    for (int i = 0; i < MAX_NUMBER_OF_DEVICES; i++)
    {
        enOcean.configureDeviceBaseID(device[i], i);
    }

#ifdef KDEBUG_min
    if (knx.configured())
    {
        if (enOcean.getNumberDevices() != MAX_NUMBER_OF_DEVICES)
            SERIAL_PORT.println(F("!!! NUMBER OF DEVICES != MAX DEVICES -> change!!!"));
        else
            SERIAL_PORT.println(F("Ready for normal operation"));
    }
#endif

#ifdef LED_YELLOW_PIN
    digitalWrite(LED_YELLOW_PIN, LOW);
#endif
}

void loop()
{
#ifdef KNXenable
    // don't delay here to much. Otherwise you might lose packages or mess up the timing with ETS
    knx.loop();
#endif
    // only run the application code if the device was configured with ETS
#ifdef KNXenable    
    if (knx.configured())
#endif    
        appLoop();
}