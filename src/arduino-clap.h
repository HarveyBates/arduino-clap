#ifndef ARDUINO_CLAP_H
#define ARDUINO_CLAP_H

#include <Arduino.h>

static char cmd_buffer[100]{};

class Arguments {
public:
    virtual ~Arguments() = default;
    virtual void call_callback(const char* arg_val) = 0;

    virtual const char* get_name() = 0;
    virtual uint8_t get_name_len() = 0;
    virtual const char* get_help() = 0;
};

namespace ParseArg {
    template <typename X>
    X type(const char* value) { return value; }

    template<>
    float type(const char* value){
        return (float)strtod(value, nullptr);
    }

    template<>
    double type(const char* value){
        return strtod(value, nullptr);
    }

    template<>
    uint32_t type(const char* value){
        long v = strtol(value, nullptr, 10);
        return v;
    }

    template<>
    uint16_t type(const char* arg_value){
        long v = strtol(arg_value, nullptr, 10);
        if(v > UINT16_MAX || v < 0){ return 0; }
        return (uint16_t)v;
    }

    template<>
    uint8_t type(const char* value){
        long v = strtol(value, nullptr, 10);
        if(v > UINT8_MAX || v < 0){ return 0; }
        return (int8_t)v;
    }

    template<>
    int32_t type(const char* value){
        long v = strtol(value, nullptr, 10);
        if(v > INT32_MAX || v < -INT32_MAX){ return 0; }
        return (int32_t)v;
    }

    template<>
    int16_t type(const char* value){
        long v = strtol(value, nullptr, 10);
        if(v > INT16_MAX || v < -INT16_MAX){ return 0; }
        return (int16_t)v;
    }

    template<>
    int8_t type(const char* value){
        long v = strtol(value, nullptr, 10);
        if(v > INT8_MAX || v < -INT8_MAX){ return 0; }
        return (int8_t)v;
    }
}

template <typename T>
class Arg : public Arguments {
    // Constants
    static const uint8_t MAX_ARG_LEN        =   8;  // Arg name length
    static const uint8_t MAX_HELP_LEN       =   50; // Help information length

    // Full name
    char name[MAX_ARG_LEN]{}; // Full name
    // Help information
    char help[MAX_HELP_LEN]{}; // Help information

public:

    typedef void(*Callback)(T);
    // Command that has a callback function attached to it
    Arg(const char* _name,
        const char* _help,
        Callback cb) :  callback(cb) {
        if(!validate_arg(_name, _help)) {
            return;
        }
        strncpy(name, _name, MAX_ARG_LEN);
        strncpy(help, _help, MAX_HELP_LEN);
    }

    Callback callback = nullptr;

    void call_callback(const char* arg_val) override {
        T value = ParseArg::type<T>(arg_val);
        callback(value);
    }

    const char* get_name() override { return name; };
    uint8_t get_name_len() override { return strlen(name); };
    const char* get_help() override { return help; }

private:
    // Ensure command name and help information are valid
    bool validate_arg(const char* _name, const char* _help_info){
        if(strlen(_name) > MAX_ARG_LEN || strlen(_help_info) > MAX_HELP_LEN){
            return false;
        }
        return true;
    }
};

class ArduinoCLI {
    Stream& stream;
    Arguments* args[10]{};
    uint8_t n_args = 0;
public:
    explicit ArduinoCLI(Stream& _serial) : stream(_serial) {}
    ~ArduinoCLI() {
        for(uint8_t i = 0; i < n_args; i++){
            free(args[i]);
        }
    }

    template <typename T>
    void add_argument(const char* name, const char* help,
                      typename Arg<T>::Callback cb){
        args[n_args++] = new Arg<T>(name, help, cb);
    }

    typedef enum {
        CLI_OK,
        CLI_UNKNOWN_COMMAND,
        CLI_PARSE_ERROR,
        CLI_HELP_NOT_FOUND,
        CLI_EXPECTED_VALUE_NOT_FOUND,
        CLI_HELP_OK
    } CLI_Status;

    void handle_error(const char* input, CLI_Status status) {
        switch(status){
            case CLI_OK:
                return;
            case CLI_UNKNOWN_COMMAND:
                stream.print("Unknown command: ");
                break;
            case CLI_EXPECTED_VALUE_NOT_FOUND:
                stream.println("Expected value not found.");
                return;
            case CLI_PARSE_ERROR:
                stream.print("ParseArg error for command: ");
                break;
            case CLI_HELP_NOT_FOUND:
                stream.println("Help information not available.");
                return;
            default:
                return;
        }
        stream.println(input);
    }

    void help(){
        stream.println("OPTIONS:");
        for(uint8_t i = 0; i < n_args; i++){
            stream.print("\t");
            stream.print(args[i]->get_name());
            stream.print("\t");
            stream.println(args[i]->get_help());
        }
    }

    CLI_Status scan_arg(char* input){
        if(!input){
            handle_error(input, CLI_EXPECTED_VALUE_NOT_FOUND);
            return CLI_EXPECTED_VALUE_NOT_FOUND;
        }

        if(strcmp(input, "help") == 0){
            help();
            return CLI_HELP_OK;
        }

        for(uint8_t i = 0; i < n_args; i++){
            if(strcmp(input, args[i]->get_name()) == 0){
                if(input[args[i]->get_name_len() - 1] == ':'){
                    input = strtok(nullptr, "\"");
                } else {
                    input = strtok(nullptr, " ");
                }

                if(!input){
                    handle_error(input, CLI_EXPECTED_VALUE_NOT_FOUND);
                }
                args[i]->call_callback(input);
                return CLI_OK;
            }
        }

        handle_error(input, CLI_UNKNOWN_COMMAND);
        return CLI_UNKNOWN_COMMAND;
    }

    bool parse_command(char* pCommand){
        char* input = strtok(pCommand, " ");

        if(exit(input)){
            memset(cmd_buffer, 0, sizeof(cmd_buffer));
            return true;
        }

        while(input){
            if(scan_arg(input) != CLI_OK){
                goto cmd_complete;
            }
            input = strtok(nullptr, " ");
        }

        cmd_complete:
        stream.print("$ ");
        memset(cmd_buffer, 0, sizeof(cmd_buffer));

        return false;
    }

    void enter(){
        stream.print("$ ");
        bool exit = false;
        while(!exit) {
            if(stream.available()){
                stream.readBytesUntil('\n', cmd_buffer, sizeof(cmd_buffer));
                cmd_buffer[strcspn(cmd_buffer, "\r\n")] = '\0';
                stream.println(cmd_buffer);
                exit = parse_command(cmd_buffer);
            }
            delay(1);
        }
    }

    bool exit(const char* input){
        if(strcmp(input, "exit") == 0){
            stream.println("Exiting command line.");
            return true;
        }
        return false;
    }
};

#endif // ARDUINO_CLAP_H
