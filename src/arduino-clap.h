#ifndef ARDUINO_CLAP_H
#define ARDUINO_CLAP_H

#include <Arduino.h>

#define CLI_RANGE_LOOP

static char cmd_buffer[100]{};

class Arguments {
public:
    virtual void execute_callback(const char* arg_val) = 0;
    virtual const char* get_name() = 0;
    virtual uint8_t get_name_len() = 0;
    virtual const char* get_help() = 0;
    virtual bool void_callback() = 0;
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
class Argument : public Arguments {
    // Constants
    static const uint8_t MAX_ARG_LEN        =   15;  // Argument name length
    static const uint8_t MAX_HELP_LEN       =   75; // Help information length
    // Full name
    char name[MAX_ARG_LEN]{}; // Full name
    // Help information
    char help[MAX_HELP_LEN]{}; // Help information
    bool void_cb = false;

public:
    typedef void(*Callback)(T);
    Callback callback = nullptr;
    // Command that has a callback function attached to it
    Argument(const char* _name, const char* _help, Callback cb) : callback(cb) {
        if(!validate_arg(_name, _help)) {
            return;
        }
        strncpy(name, _name, MAX_ARG_LEN);
        strncpy(help, _help, MAX_HELP_LEN);
    }

    typedef void(*vCallback)();
    vCallback vcallback = nullptr;
    Argument(const char* _name, const char* _help, vCallback cb) : vcallback(cb) {
        if(!validate_arg(_name, _help)) {
            return;
        }
        void_cb = true;
        strncpy(name, _name, MAX_ARG_LEN);
        strncpy(help, _help, MAX_HELP_LEN);
    }

    void execute_callback(const char* arg_val) override {
        if(void_cb){
            vcallback();
            return;
        }
        T value = ParseArg::type<T>(arg_val);
        callback(value);
    }

    const char* get_name() override { return name; }
    uint8_t get_name_len() override { return strlen(name); }
    const char* get_help() override { return help; }
    bool void_callback() override { return void_cb; }

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
    char help_buffer[100]{}; // Single line of help message
public:
    explicit ArduinoCLI(Stream& _serial) : stream(_serial) {}
    ~ArduinoCLI() {
        for(uint8_t i = 0; i < n_args; i++){
            free(args[i]);
        }
    }

    template <typename T = void>
    void add_argument(const char* name, const char* help,
                      void(*cb)(T)){
        args[n_args++] = new Argument<T>(name, help, cb);
    }

    template <typename T = uint8_t>
    void add_argument(const char* name, const char* help,
                      void(*cb)()){
        args[n_args++] = new Argument<T>(name, help, cb);
    }

    typedef enum {
        CLI_OK,
        CLI_ERROR,
        CLI_UNKNOWN_COMMAND,
        CLI_HELP_OK,
        CLI_EXPECTED_VALUE_NOT_FOUND,
    } CLI_Status;

private:
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
            default:
                return;
        }
        stream.println(input);
    }

    void help(){
        stream.println("OPTIONS:");
        for(uint8_t i = 0; i < n_args; i++){
            inbuilt_help(args[i]->get_name(), args[i]->get_help());
        }
        stream.println("HELPERS:");
        inbuilt_help("help", "Print out help information.");
#ifdef CLI_RANGE_LOOP
        inbuilt_help("range", "Execute function with values within a range "
                              "(start:stop:interval_ms).");
        inbuilt_help("loop", "Execute function in loop with values "
                              "(start:stop:interval_ms).");
        inbuilt_help("stop", "Stop range or loop function.");
#endif // CLI_RANGE_LOOP
        inbuilt_help("exit", "Exit CLI cleanly.");
    }

    void inbuilt_help(const char* _name, const char* _help){
        snprintf(help_buffer, sizeof(help_buffer), "\t%-20s%-80s",
                 _name, _help);
        stream.println(help_buffer);
        memset(help_buffer, 0, sizeof(help_buffer));
    }

#ifdef CLI_RANGE_LOOP
    CLI_Status parse_range_loop(char* input, Arguments* arg){

        bool _range = true;

        if(strcmp("loop", input) == 0){
            _range = false;
        }

        input = strtok(nullptr, " ");

        if(!input){
            handle_error(input, CLI_EXPECTED_VALUE_NOT_FOUND);
            return CLI_EXPECTED_VALUE_NOT_FOUND;
        }

        char start_buf[6];
        char stop_buf[6];
        char interval_buf[10];

        sscanf(input, "%5[^:]:%5[^:]:%9s", start_buf, stop_buf, interval_buf);

        int32_t start = ParseArg::type<int32_t>(start_buf);
        int32_t stop = ParseArg::type<int32_t>(stop_buf);
        uint32_t interval = ParseArg::type<uint32_t>(interval_buf);

        if(start > stop){
            return CLI_ERROR;
        }

        if(_range){
            cb_range(arg, start, stop, interval);
        } else {
            cb_loop(arg, start, stop, interval);
        }

        return CLI_OK;
    }

    void cb_range(Arguments* arg, int32_t start, int32_t stop,
                         uint32_t interval){
        char range_buf[6]{};
        for(int32_t i = start; i <= stop; i++){
            if(range_loop_exit()){
                return;
            }
            snprintf(range_buf, sizeof(range_buf), "%ld", i);
            arg->execute_callback(range_buf);
            memset(range_buf, 0, sizeof(range_buf));
            delay(interval);
        }

    }

    void cb_loop(Arguments* arg, int32_t start, int32_t stop,
                 uint32_t interval) {
        char range_buf[6]{};
        while (true) {
            for (int32_t i = start; i <= stop; i++) {
                if(range_loop_exit()) { return; }
                snprintf(range_buf, sizeof(range_buf), "%ld", i);
                arg->execute_callback(range_buf);
                memset(range_buf, 0, sizeof(range_buf));
                delay(interval);
            }
            for (int32_t i = stop-1; i >= start+1; i--) {
                if(range_loop_exit()){ return; }
                snprintf(range_buf, sizeof(range_buf), "%ld", i);
                arg->execute_callback(range_buf);
                memset(range_buf, 0, sizeof(range_buf));
                delay(interval);
            }
        }
    }

    bool range_loop_exit(){
        if(stream.available()){
            stream.readBytesUntil('\n', cmd_buffer, sizeof(cmd_buffer)-1);
            cmd_buffer[strcspn(cmd_buffer, "\r\n")] = '\0';
            if(strcmp("stop", cmd_buffer) == 0){
                return true;
            }
        }
        return false;
    }
#endif // CLI_RANGE_LOOP


    CLI_Status scan_arg(char* input){
        if(!input){
            handle_error(input, CLI_EXPECTED_VALUE_NOT_FOUND);
            return CLI_EXPECTED_VALUE_NOT_FOUND;
        }

        if(strcmp("help", input) == 0){
            help();
            return CLI_HELP_OK;
        }

        for(uint8_t i = 0; i < n_args; i++){
            if(strcmp(args[i]->get_name(), input) == 0){
                if(args[i]->void_callback()){
                    args[i]->execute_callback(input);
                    return CLI_OK;
                }

                if(input[args[i]->get_name_len() - 1] == ':'){
                    input = strtok(nullptr, "\"");
                } else {
                    input = strtok(nullptr, " ");
                }

                if(!input){
                    handle_error(input, CLI_EXPECTED_VALUE_NOT_FOUND);
                }

                #ifdef CLI_RANGE_LOOP
                if(strcmp("range", input) == 0 || strcmp("loop", input) == 0){
                    return parse_range_loop(input, args[i]);
                }
                #endif

                args[i]->execute_callback(input);
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

    bool exit(const char* input){
        if(strcmp("exit", input) == 0){
            stream.println("Exited command line.");
            return true;
        }
        return false;
    }

public:

    void enter(){
        stream.print("$ ");
        bool exit = false;
        while(!exit) {
            if(stream.available()){
                stream.readBytesUntil('\n', cmd_buffer, sizeof(cmd_buffer)-1);
                cmd_buffer[strcspn(cmd_buffer, "\r\n")] = '\0';
                stream.println(cmd_buffer);
                exit = parse_command(cmd_buffer);
            }
            delay(1);
        }
    }
};

#endif // ARDUINO_CLAP_H
