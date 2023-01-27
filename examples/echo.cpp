#include <arduino_clap.h>

// Setup command line interface objects
ArduinoCLI* cli;

void echo(const char* input){
    Serial.println(input);
}

void setup(){
    Serial.begin(115200);

    // Pass the serial object into the CLI
    cli = new ArduinoCLI(Serial);

    // New echo command
    cli->add_argument<const char*>("echo:", "Echo user input.", echo);

    // Enter the CLI
    cli->enter();

    // echo: "Hello World!"
}

void loop(){
    // Wont get here until the CLI (cli) is exited with the "exit" command
    delay(1);
}
