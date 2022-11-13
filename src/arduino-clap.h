#ifndef ARDUINO_CLAP_H
#define ARDUINO_CLAP_H

#include <Arduino.h>

class CL_Command{
    // Constants
    static const uint8_t MAX_ARG_LEN        =       20;
    static const uint8_t MAX_NEXTED_DEPTH   =       10;
    static const uint8_t MAX_HELP_LEN       =       UINT8_MAX;

public:
    // Commands with function callback
    CL_Command(const char* _name, bool _takes_value, const char* _help_info,
               void (*_callback)());
    // Commands with nested command
    CL_Command(const char* _name, const char* _help_info,
               CL_Command* _nested_command);
    // Command with multiple nested command
    CL_Command(const char* _name, const char* _help_info);

    // Add a sub command to an existing command
    void add_sub_command(CL_Command* _nested_command);

    // Full name
    char name[MAX_ARG_LEN]{}; // Full name
    // Has a value passed to it
    bool takes_value = false; // Has value
    // Help information
    char help[MAX_HELP_LEN]{}; // Help information
    // Has a sub-command
    bool has_child = false; // Has at least one sub-command
    // Number of nested commands
    uint8_t n_sub_commands = 0;
    // List of nested commands
    CL_Command* sub_commands[MAX_NEXTED_DEPTH]{};
    // Command callback function
    void (*callback)(){};
};

class ArduinoCLI {
    // Constants
    static const uint8_t MAX_NAME_LEN       =       20;
    static const uint8_t MAX_COMMANDS       =       3;

    uint8_t n_commands                      =       0;
    CL_Command* commands[MAX_COMMANDS]{};

    Stream& serial;

public:
    explicit ArduinoCLI(Stream& _serial) : serial(_serial) {}
    ~ArduinoCLI() = default;

    static const uint8_t MAX_CMD_BUF_LEN    =       UINT8_MAX;

    void add_command(CL_Command* command);
    CL_Command* scan_commands(const char* input);
    CL_Command* scan_command(const char* input,
                             const CL_Command* parent_command);
    void help(const CL_Command* command);
    void help();
    bool exit(const char* input);
    void enter();
};


#endif //ARDUINO_CLAP_H
