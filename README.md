# arduino-clap
Arduino ecosystem command line argument parser (CLAP).

## Features
A simple header only CLI to set random function values on the fly.
- Small implementation (500b to 1kb).
- Automatically coverts the CLI arguments to their type so you don't have to do that yourself. 
- Supports multiple args on a single line.
- Supports args in quotations.
- Has an input range function, e.g. `speed range 0:100:500` (start:stop:interval)
- Inbuilt help.

# Usage

## Example
```c++
#include <arduino-clap.h>

void echo(const char* msg){
    Serial.println(msg);
}

void set_motor_speed(uint16_t speed){
    update_speed(speed);
}

ArduinoCLI* cli;

void setup(){
    Serial.begin(115200);
    cli = new ArduinoCLI(Serial);
    cli->add_argument<const char*>("echo:", 
                                   "Echo user input.", 
                                   echo);
    cli->add_argument<uint16_t>("motor_speed", 
                                "Set motor speed.", 
                                set_motor_speed);
    cli->enter();
}

void loop(){
    // Will not get here until the CLI (cli) is exited with 
    // the "exit" command
    delay(1);
}
```

## Running the above example
```
$ echo: "Hello World!"
Hello World!

$ motor_speed 9000
-> Motor speed set to 9000

$ motor_speed range 0:100:500
-> Motor speed set from 0 to 100 with 500 ms between speed changes
```

## Inbuilt help
The example above has an inbuilt help function.

```
$ help
OPTIONS:
    echo:               echo user input                                   
    motor_speed         Set motor speed                                    
    int                 echo user input    
```

## Exiting
```
$ exit
Exited command line.
```
