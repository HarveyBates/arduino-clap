#include <arduino-clap.h>

// Setup command objects
Argument* echo_commands;

// Setup command line interface objects
ArduinoCLI* cli;

void echo(const char* input){
    Serial.println(input);
}

void setup(){
    Serial.begin(115200);
    pinMode(BUILTIN_LED, OUTPUT);

    // Pass the serial object into the CLI
    cli = new ArduinoCLI(Serial);

    // New echo command
    echo_commands = new Argument("echo", "Echo user input", echo, true);

    // Add "echo" command to cli
    cli->add_command(echo_commands);

    // Enter the CLI
    cli->enter();

    free(echo_commands);
    free(cli);
}

void loop(){
    // Wont get here until the CLI (cli) is exited with the "exit" command
    delay(1);
}