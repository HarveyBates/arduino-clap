#include <arduino_clap.h>

// Setup command line interface objects
ArduinoCLI* cli;

// Basic functions to demonstrate usage
void blink_dynamic(uint16_t rate){
    for(uint8_t i = 0; i < 10; i++){
        digitalWrite(BUILTIN_LED, HIGH);
        delay(rate);
        digitalWrite(BUILTIN_LED, LOW);
        delay(rate);
    }
}

void setup(){
    Serial.begin(115200);
    pinMode(BUILTIN_LED, OUTPUT);

    // Pass the serial object into the CLI
    cli = new ArduinoCLI(Serial);

    cli->add_argument<uint16_t>("blink-dyn", 
        "Blink the onboard LED dynamically!", blink_dynamic);

    // Enter the CLI
    cli->enter();

    // In the CLI:
    // blink-dyn 50
}

void loop(){
    // Wont get here until the CLI (cli) is exited with 
    // the "exit" command
    delay(1);
}
