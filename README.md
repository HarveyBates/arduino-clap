# arduino-clap

A header only CLI to set function values during Arduino prototyping.

## Features

### Small size
Developed for the Arduino UNO and above.

### Automatic type conversion
Converts arguments to the type required by the function. For example:
```c++
void set_speed(int _speed){ speed = _speed; }
...
cli->add_argument<int>("speed", "Set motor speed", set_speed);
cli->add_argument<float>("direction", "Set compass direction", set_direction);
...
```
Can be called within the CLI as:
```bash
speed 100
```
If the value exceeds the maximum value of that type it is reverted to `0`. For example:
```bash
speed 90000 # Returns 0 (as int has a max value of 32767)
```

### Multiple args on single line
Arguments are processed inline, so you can do the following:
```c++
void enable(){ motor_on = true; }
void set_direction(float _direction){ direction = _direction; }
void set_speed(int _speed){ speed = _speed; }
...
cli->add_argument("enable", "Turn on motor", enable); // void CLI argument (accepts no value)
cli->add_argument<int>("speed", "Set motor speed", set_speed);
cli->add_argument<float>("direction", "Set compass direction", set_direction);
```
Then in the CLI:
```
enable speed 150 direction 98.2
```

### Space delimited strings
Character arrays can be surrounded in quotes if they have spaces or alone if a single 
phrase is used. For example:
```c++
void echo(const char* msg) { Serial.println(msg); }
...
cli->add_argument<const char*>("echo", "Echo user input", echo);
```
Within the CLI:
```bash
echo "Hello World!"
# Output = Hello World!
echo Hello 
# Output = Hello
```

### Inbuilt Arduino Helpers
Three helper functions are provided, `range` `loop` and `array`. The first two are for integer types only (`array` accepts floats and doubles). The command `stop` can be used to stop a `range` or `loop` function.
#### Range
Executes a function with values provided between a range with spacing set by an interval. For example:
```c++
void update_speed(int _speed) { speed = _speed };
...
cli->add_argument<int>("speed", "Update motor speed", update_speed);
```
Then to call to set speeds between a range within the CLI:
```bash
speed range 0:100:500 # start:stop:interval (in ms)
```
This will call the `update_speed()` function with the values 0 to 100 with a new call every 500 ms.

#### Loop
Based on the same example a in the `range` function, the `loop` function will continually loop through the `range` (forwards and backwards) until the `stop` command has been provided by the user.
```bash
speed loop 0:100:500
```
This will call the `update_speed()` function with the values 0 to 100 then 100 to 0 with a new call every 500 ms.

#### Array
The `array` function as the name suggests takes an array of abritraty values and passes them to a function sequentially, spaced by a user defined interval. 
```bash
speed array 1000:[10, 2, 3, 81, 77] // Using ints
direction array 1000:[8.2, 71.3, 110.1, 22.6] // Using floats or doubles
```
The above example will pass the values found in the `[]` to the "speed" callback function with a delay of 1000 ms between each call.

## Example
```c++
#include <arduino-clap.h>
#include <Servo.h> // Not necessary, just an example

void echo(const char* msg){ Serial.println(msg); }
void set_motor_speed(int speed){ update_speed(speed); }

Servo servo;

ArduinoCLI* cli;

void setup(){
    Serial.begin(115200);
    servo.attach(9);
   
    cli = new ArduinoCLI(Serial);
    cli->add_argument<const char*>("echo", "Echo user input.", echo);
    cli->add_argument<int>("motor-speed", "Set motor speed.", set_motor_speed);
    cli->add_argument<int>("servo-angle", "Set servo angle.", [](int a){ servo.write(a); }); // Non-static
    cli->enter();
}

void loop(){
    // Will not get here until the CLI (cli) is exited with 
    // the "exit" command
    delay(1);
}
```

## Inbuilt help
The example above has an inbuilt help function.

```
$ help
OPTIONS:
	echo                Echo user input.                                                 
	motor-speed         Set motor speed.
	servo-angle.        Set servo angle.
HELPERS:
	help                Print out help information.                                                   
	range               Execute function with values within a range (start:stop:interval_ms).         
	loop                Execute function in loop with values (start:stop:interval_ms).                
	stop                Stop range or loop function.                                                  
	exit                Exit CLI cleanly.                                                             
```

## Exiting
```
$ exit
Exited command line.
```

## Licence 
This project is under the GNU LESSER GENERAL PUBLIC LICENSE as found in the LICENCE file.
