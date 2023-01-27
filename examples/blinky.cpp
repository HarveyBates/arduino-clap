#include <arduino_clap.h>

// Setup command line interface objects
ArduinoCLI* cli;

// Basic functions to demonstrate usage
void blink_fast(){
    Serial.println("Blinking fast!");
    for(uint8_t i = 0; i < 10; i++){
        digitalWrite(BUILTIN_LED, HIGH);
        delay(100);
        digitalWrite(BUILTIN_LED, LOW);
        delay(100);
    }
}

// Basic functions to demonstrate usage
void blink_slow(){
    Serial.println("Blinking slow!");
    for(uint8_t i = 0; i < 10; i++){
        digitalWrite(BUILTIN_LED, HIGH);
        delay(500);
        digitalWrite(BUILTIN_LED, LOW);
        delay(500);
    }
}

void setup(){
    Serial.begin(115200);
    pinMode(BUILTIN_LED, OUTPUT);

    // Pass the serial object into the CLI
    cli = new ArduinoCLI(Serial);

    cli->add_argument("blink-fast", "Blink the onboard LED fast!", blink_fast);
    cli->add_argument("blink-slow", "Blink the onboard LED slow!", blink_slow);

    // Enter the CLI
    cli->enter();

    // In the CLI:
    // blink-fast
    // blink-slow
}

void loop(){
    // Wont get here until the CLI (cli) is exited with the "exit" command
    delay(1);
}
