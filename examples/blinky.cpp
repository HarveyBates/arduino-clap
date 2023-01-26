#include <arduino-clap.h>

// Setup command objects
Argument* led_blink_fast;
Argument* led_blink_slow;
Argument* led_blink;
Argument* led_commands;

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

    // Add a blink fast command with a callback to the "void blink_fast()" function
    led_blink_fast = new Argument("fast", "Blink the onboard LED fast!", blink_fast);
    led_blink_slow = new Argument("slow", "Blink the onboard LED slow!", blink_slow);

    // Command for controlling the onboard led's blink rate
    led_blink = new Argument("blink", "LED blink rate control");
    // Append the "fast" and "slow" commands to the "blink" command
    led_blink->add_sub_command(led_blink_fast);
    led_blink->add_sub_command(led_blink_slow);

    // Command for LED control functions
    led_commands = new Argument("led", "LED control service");
    // Append the "blink" command to the "led" command
    led_commands->add_sub_command(led_blink);

    // Add the "led" command to the CLI
    cli->add_command(led_commands);

    // Enter the CLI
    cli->enter();

    free(led_blink_fast);
    free(led_blink_slow);
    free(led_blink);
    free(led_commands);
    free(cli);
}

void loop(){
    // Wont get here until the CLI (cli) is exited with the "exit" command
    delay(1);
}