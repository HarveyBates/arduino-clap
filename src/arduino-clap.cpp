#include "arduino-clap.h"

static char cmd_buffer[ArduinoCLI::MAX_CMD_BUF_LEN];

CL_Command::CL_Command(const char* _name, const char* _help_info,
                       void (*_callback)(), bool _takes_value){
    begin(_name, _help_info);
    takes_value = _takes_value;
    callback = _callback;
    callback_type = none;
}

CL_Command::CL_Command(const char *_name, const char *_help_info,
                       void (*_callback)(int32_t), bool _takes_value){
    begin(_name, _help_info);
    takes_value = _takes_value;
    i32_callback = _callback;
    callback_type = i32;
}

CL_Command::CL_Command(const char *_name, const char *_help_info,
                       void (*_callback)(const char*), bool _takes_value) {
    begin(_name, _help_info);
    takes_value = _takes_value;
    str_callback = _callback;
    callback_type = str;
}

CL_Command::CL_Command(const char* _name, const char* _help_info){
    begin(_name, _help_info);
    callback_type = none;
}

void CL_Command::begin(const char* _name, const char* _help_info){
    assert(strlen(_name) < MAX_ARG_LEN);
    assert(strlen(_help_info) < MAX_HELP_LEN);

    strncpy(name, _name, MAX_ARG_LEN);
    strncpy(help, _help_info, MAX_HELP_LEN);
}

void CL_Command::add_sub_command(CL_Command* _nested_command){
    has_child = true;

    if(n_sub_commands < MAX_NEXTED_DEPTH){
        sub_commands[n_sub_commands++] = _nested_command;
    }
}

void ArduinoCLI::add_command(CL_Command *command) {
    if(n_commands < MAX_COMMANDS){
        commands[n_commands++] = command;
    }
}

CL_Command* ArduinoCLI::scan_primary_commands(const char* input){

    if(strcmp(input, "help") == 0){
        help();
        return nullptr;
    }

    for(uint8_t i = 0; i < n_commands; i++){
        if(strcmp(input, commands[i]->name) == 0){
            if(!handle_command(input, commands[i])){
                // Command still has children
                return commands[i];
            }
            // Command complete
            return nullptr;
        }
    }

    handle_error(input, CLI_UNKNOWN_COMMAND);
    return nullptr;
}

CL_Command* ArduinoCLI::scan_sub_commands(const char* input,
                                          CL_Command* current_command){

    if(strcmp(input, "help") == 0){
        help(current_command);
        return nullptr;
    }

    if(!current_command){
        handle_error(input, CLI_PARSE_ERROR);
        return nullptr;
    }

    for(uint8_t i = 0; i < current_command->n_sub_commands; i++){
        if(strcmp(input, current_command->sub_commands[i]->name) == 0){
            if(!handle_command(input, current_command->sub_commands[i])){
                // Command still has children
                return current_command->sub_commands[i];
            }
            // Command complete
            return nullptr;
        }
    }

    handle_error(input, CLI_UNKNOWN_COMMAND);
    return nullptr;
}

bool ArduinoCLI::handle_command(const char *input, CL_Command* command) {
    if(!command->has_child){
        switch(command->callback_type){
            case CL_Command::CallbackType::i32:{
                int32_t i32_value = get_i32_value();
                if(i32_value){
                    command->i32_callback(i32_value);
                } else {
                    handle_error(input, CLI_PARSE_ERROR);
                }
                break;
            }
            case CL_Command::CallbackType::str:{
                char* value = strtok(nullptr, "\"");
                if(value){
                    command->str_callback(value);
                } else {
                    handle_error(input, CLI_EXPECTED_VALUE_NOT_FOUND);
                }
                break;
            }
            case CL_Command::none:
                command->callback();
                break;
        }
        return true;
    }

    return false;
}


int32_t ArduinoCLI::get_i32_value(){
    const char* value = strtok(nullptr, " ");

    char* remainder = nullptr;

    if(!value){
        serial.println("Error: value not found.");
    } else {
        int32_t i32_value = strtol(value, &remainder, 10);
        if(remainder == value) {
            // Error parsing value
            return NULL;
        } else {
            return i32_value;
        }
    }

    return NULL;
}

void ArduinoCLI::handle_error(const char* input, CLI_Status status) {
    switch(status){
        case CLI_OK:
            return;
        case CLI_UNKNOWN_COMMAND:
            serial.print("Unknown command: ");
            break;
        case CLI_EXPECTED_VALUE_NOT_FOUND:
            serial.println("Expected value not found.");
            return;
        case CLI_PARSE_ERROR:
            serial.print("Parse error for command: ");
            break;
        case CLI_HELP_NOT_FOUND:
            serial.println("Help information not available.");
            return;
        case CLI_COMPLETE:
            return;
    }
    serial.println(input);
}

void ArduinoCLI::help(const CL_Command* command){
    serial.println("USAGE:");
    serial.print("\t");
    serial.print(command->name);
    serial.println(" [OPTIONS]");

    if(command->n_sub_commands == 0){
        handle_error(command->name, CLI_HELP_NOT_FOUND);
        return;
    }

    serial.println("OPTIONS:");
    for(uint8_t i = 0; i < command->n_sub_commands; i++){
        serial.print("\t");
        serial.print(command->sub_commands[i]->name);
        serial.print("\t");
        serial.println(command->sub_commands[i]->help);
    }
}

void ArduinoCLI::help(){
    serial.println("OPTIONS:");
    for(uint8_t i = 0; i < n_commands; i++){
        serial.print("\t");
        serial.print(commands[i]->name);
        serial.print("\t");
        serial.println(commands[i]->help);
    }
}

bool ArduinoCLI::exit(const char* input){
    if(strcmp(input, "exit") == 0){
        serial.println("Exiting command line.");
        return true;
    }
    return false;
}

void ArduinoCLI::enter(){
    serial.print("$ ");
    while(true) {
        if(serial.available()){
            serial.readBytesUntil('\n', cmd_buffer, sizeof(cmd_buffer));
            serial.println(cmd_buffer);

            char* input = strtok(cmd_buffer, " ");

            if(exit(input)){
                memset(cmd_buffer, 0, sizeof(cmd_buffer));
                return;
            }

            CL_Command* current_command = scan_primary_commands(input);

            if(!current_command){
                goto cmd_complete;
            }

            input = strtok(nullptr, " ");

            while(input){
                current_command = scan_sub_commands(input, current_command);
                if(!current_command){
                    goto cmd_complete;
                }
                input = strtok(nullptr, " ");
            }

            cmd_complete:
            serial.print("$ ");
            memset(cmd_buffer, 0, sizeof(cmd_buffer));
        }
        delay(1);
    }
}