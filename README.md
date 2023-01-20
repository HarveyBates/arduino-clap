# arduino-clap
Arduino ecosystem command line argument parser (CLAP).

# Usage

## Example
```c++
#include <arduino-clap.h>

// Setup LED control commands
CL_Command* led_blink_fast;
CL_Command* led_blink_slow;
CL_Command* led_blink_dyn;
CL_Command* led_blink;
CL_Command* led_commands;

// Setup echo command
CL_Command* echo_commands;

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

// Basic function using user input (int32_t)
void blink_dynamic(int32_t rate){
    for(uint8_t i = 0; i < 10; i++){
        digitalWrite(BUILTIN_LED, HIGH);
        delay(rate);
        digitalWrite(BUILTIN_LED, LOW);
        delay(rate);
    }
}

// Function that echos user input
void echo(const char* input){
    Serial.print(input);
}

void setup(){
    Serial.begin(115200);
    pinMode(BUILTIN_LED, OUTPUT);

    // Pass the serial object into the CLI
    cli = new ArduinoCLI(Serial);

    // Add a blink fast command with a callback to the "void blink_fast()" function
    led_blink_fast = new CL_Command("fast", "Blink the onboard LED fast!", blink_fast);
    led_blink_slow = new CL_Command("slow", "Blink the onboard LED slow!", blink_slow);
    
    // Add a dynamic blinker (type inferred from constructor)
    led_blink_dyn = new CL_Command("dynamic", "Blink the onboard LED dynamically!",
                                    blink_dynamic, true);

    // Command for controlling the onboard led's blink rate
    led_blink = new CL_Command("blink", "LED blink rate control");
    // Append the "fast" and "slow" commands to the "blink" command
    led_blink->add_sub_command(led_blink_fast);
    led_blink->add_sub_command(led_blink_slow);
    led_blink->add_sub_command(led_blink_dyn);

    // Command for LED control functions
    led_commands = new CL_Command("led", "LED control service");
    // Append the "blink" command to the "led" command
    led_commands->add_sub_command(led_blink);
    
    // Echo input commands
    echo_commands = new CL_Command("echo", "Echo user input", echo, true);

    // Add the "led" command to the CLI
    cli->add_command(led_commands);
    cli->add_command(echo_commmands);

    // Enter the CLI
    cli->enter();
    
    free(led_blink_fast);
    free(led_blink_dyn);
    free(led_blink_str);
    free(led_blink);
    free(led_commands);
    free(echo_commands);
    free(cli);
}

void loop(){
    // Wont get here until the CLI (cli) is exited with the "exit" command
    delay(1);
}
```

## Running the above example
```
$ led blink slow
Blinking Slow!

$ led blink fast
Blinking Fast!

$ led blink dynamic 200
-> LED blinking with 200 ms spacing

$ echo "this is some user input"
this is some user input
```

## Inbuilt help
The example above has an inbuilt help function for each level.

```
$ help
OPTIONS:
	led	Onboard LED control service
	echo	Echo user input
```

```
$ led help
USAGE:
	led [OPTIONS]
OPTIONS:
	blink	LED blink rate control.
```

```
$ led blink help
USAGE:
	blink [OPTIONS]
OPTIONS:
	fast	Blink the onboard LED fast!
	slow	Blink the onboard LED slow!
	dynamic	Blink the onboard LED dynamically!
```

## Exiting
```
$ exit
Exiting command line.
```

## Limitations
1. Class functions must be static.
2. Input command size has a maximum of 256 chacracters.
3. Maxiumum length of an argument is 8 chacracters.
4. There can be a maximum of 3 nested arguments.
5. Maxiumum length of a help string is 100 characters.

