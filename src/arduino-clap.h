#ifndef ARDUINO_CLAP_H
#define ARDUINO_CLAP_H

#include <Arduino.h>

template <typename T, typename... U>
class CL_Command{
    // Holds function pointer to callback function
    T callback = nullptr;

    // Constants
    static const uint8_t MAX_ARG_LEN        =       8;  // Arg name character length
    static const uint8_t MAX_NESTED_DEPTH   =       3;  // Number of nested args
    static const uint8_t MAX_HELP_LEN       =       50; // Help information character length

    // Full name
    char name[MAX_ARG_LEN]{}; // Full name
    // Help information
    char help[MAX_HELP_LEN]{}; // Help information
    // Number of nested commands
    uint8_t n_commands = 0; // If not zero then there is a sub_command
    // List of nested commands
    void* sub_commands[MAX_NESTED_DEPTH]{};

public:
    // Command that has a callback function attached to it
	CL_Command(const char* _name, const char* _help, T cb = nullptr) : callback(cb) {
        if(!validate_arg(_name, _help)) {
            return;
        }
		strncpy(name, _name, MAX_ARG_LEN);
		strncpy(help, _help, MAX_HELP_LEN);
    }

    // Add a sub command to an existing command
    template <typename... V>
	void add_sub_command(CL_Command<void (*)(V...), V...>* _cmd){
		if(n_commands < MAX_NESTED_DEPTH){
			sub_commands[n_commands++] = _cmd;
		}
	}

private:
    // Ensure command name and help information are valid
    bool validate_arg(const char* _name, const char* _help_info){
        if(strlen(_name) > MAX_ARG_LEN || strlen(_help_info) > MAX_HELP_LEN){
            return false;
        }
        return true;
    }
};

#endif // ARDUINO_CLAP_H
