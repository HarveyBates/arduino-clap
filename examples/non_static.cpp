#include <arduino_clap.h>
#include <Servo.h>

// Setup command line interface objects
ArduinoCLI* cli;

// Setup servo object
Servo servo;

// Lambda to encaptulate instance of servo
auto servo_pos = [](int v){
  servo.write(v);
  Serial.print("Servo Position: ");
  Serial.println(v);
}

void setup(){
    Serial.begin(115200);

    // Attach servo to desired pin
    servo.attach(9);

    // Pass the serial object into the CLI
    cli = new ArduinoCLI(Serial);

    // Add servo position argument to CLI
    cli->add_argument<int>("servo-pos", "Set servo position.", servo_pos);

    // Enter the CLI
    cli->enter();

    // In the CLI:
    // servo-pos 180
}

void loop(){
    // Wont get here until the CLI (cli) is exited with the "exit" command
    delay(1);
}
