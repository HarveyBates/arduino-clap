#include "arduino-clap.h"

static char cmd_buffer[ArduinoCLI::MAX_CMD_BUF_LEN];

CL_Command::CL_Command(const char* _name, const char* _help_info,
                       void (*_callback)(), bool _takes_value){

    assert(strlen(_name) < MAX_ARG_LEN);
    assert(strlen(_help_info) < MAX_HELP_LEN);

    strncpy(name, _name, MAX_ARG_LEN);
    takes_value = _takes_value;
    strncpy(help, _help_info, MAX_HELP_LEN);

    callback = _callback;
}

CL_Command::CL_Command(const char* _name, const char* _help_info){
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

CL_Command* ArduinoCLI::scan_commands(const char* input){
    if(strcmp(input, "help") == 0){
        help();
        return nullptr;
    }

    for(uint8_t i = 0; i < n_commands; i++){
        if(strcmp(input, commands[i]->name) == 0){
            if(!commands[i]->has_child){
                commands[i]->callback();
            }
            return commands[i];
        }
    }

    serial.printf("Unknown service request: %s\n", input);
    return nullptr;
}

CL_Command* ArduinoCLI::scan_command(const char* input,
                                     const CL_Command* parent_command){

    if(strcmp(input, "help") == 0){
        help(parent_command);
        return nullptr;
    }

    for(uint8_t i = 0; i < parent_command->n_sub_commands; i++){
        if(strcmp(input, parent_command->sub_commands[i]->name) == 0){
            if(!parent_command->sub_commands[i]->has_child){
                parent_command->sub_commands[i]->callback();
            }
            return parent_command->sub_commands[i];
        }
    }

    serial.print("Unknown command: ");
    serial.println(input);

    return nullptr;
}

void ArduinoCLI::help(const CL_Command* command){
    serial.println("USAGE:");
    serial.print("\t");
    serial.print(command->name);
    serial.println(" [OPTIONS]");

    if(command->n_sub_commands == 0){
        serial.print("Help not available for command: ");
        serial.println(command->name);
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
    bool exit_cli = false;
    while(!exit_cli) {
        if(serial.available()){
            serial.readBytesUntil('\n', cmd_buffer, sizeof(cmd_buffer));
            serial.println(cmd_buffer);

            // TODO Needs some assertion that the command is correct

            const char* delim_cmd = strtok(cmd_buffer, " ");
            CL_Command* parent_argument = nullptr;
            while(delim_cmd != NULL){
                exit_cli = exit(delim_cmd);
                if(exit_cli){
                    break;
                }
                if(parent_argument == nullptr){
                    parent_argument = scan_commands(delim_cmd);
                    if(parent_argument == nullptr){
                        break;
                    }
                } else {
                    parent_argument = scan_command(delim_cmd,
                                                   parent_argument);
                }
                delim_cmd = strtok(NULL, " ");
            }
            serial.print("$ ");
        }

        memset(cmd_buffer, 0, sizeof(cmd_buffer));
        delay(1);
    }
}