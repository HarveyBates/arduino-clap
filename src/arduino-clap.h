#ifndef ARDUINO_CLAP_H
#define ARDUINO_CLAP_H

#include <Arduino.h>

class CL_Command{
    // Constants
    static const uint8_t MAX_ARG_LEN        =       8;
    static const uint8_t MAX_NESTED_DEPTH   =       3;
    static const uint8_t MAX_HELP_LEN       =       100;


public:
    // Commands with function callback
    CL_Command(const char* _name, const char* _help_info,
               void (*_callback)(), bool _takes_value = false);
    // Command with multiple nested commands (to be added post initialisation
    // with "add_sub_command()")
    CL_Command(const char* _name, const char* _help_info);

    CL_Command(const char *_name, const char *_help_info,
               void (*_callback)(int32_t), bool _takes_value);

    CL_Command(const char *_name, const char *_help_info,
               void (*_callback)(const char*), bool _takes_value);

    // Add a sub command to an existing command
    void add_sub_command(CL_Command* _nested_command);

    enum CallbackType {
        none    =   0x00,
        i32     =   0x01,
        str     =   0x02,
    };

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
    CL_Command* sub_commands[MAX_NESTED_DEPTH]{};
    // Command callback function (no value passed)
    void (*callback)(){};
    // Command callback with int32_t
    void (*i32_callback)(int32_t){};
    // Command callback with character array
    void (*str_callback)(const char*){};
    // Call back type for parsing
    CallbackType callback_type;

private:
    // Helper for multiple constructors with different callback value types
    void begin(const char* _name, const char* _help_info);
};

class ArduinoCLI {
    // Constants
    static const uint8_t MAX_COMMANDS       =       10;

    uint8_t n_commands                      =       0;
    CL_Command* commands[MAX_COMMANDS]{};


public:
    explicit ArduinoCLI(Stream& _serial) : serial(_serial) {}

    ~ArduinoCLI() = default;

    Stream& serial;
    static const uint16_t MAX_CMD_BUF_LEN    =       UINT8_MAX;

    typedef enum {
        CLI_OK = 0x00,
        CLI_UNKNOWN_COMMAND = 0x01,
        CLI_PARSE_ERROR = 0x02,
        CLI_HELP_NOT_FOUND = 0x03,
        CLI_EXPECTED_VALUE_NOT_FOUND = 0x04,
        CLI_COMPLETE = 0x05
    } CLI_Status;

    void add_command(CL_Command* command);
    CL_Command* scan_primary_commands(const char* input);
    CL_Command* scan_sub_commands(const char* input,
                                  CL_Command* current_command);
    bool handle_command(const char *input, CL_Command* command);
    void handle_error(const char* input, CLI_Status status);
    int32_t get_i32_value();
    void help(const CL_Command* command);
    void help();
    bool parse_command(char* pCommand);
    virtual void enter();
    bool exit(const char* input);
};


#endif //ARDUINO_CLAP_H
