#ifndef ARDUINO_CLAP_H
#define ARDUINO_CLAP_H

#include <Arduino.h>

enum CB_Type { none, _i8, _i16, _i32, _u8, _u16, _u32, _ch };

class Arguments {
public:
    virtual ~Arguments() = default;
    virtual void call_callback(const char* msg) = 0;

    virtual const char* get_name() = 0;
    virtual const char* get_help_info() = 0;
};

template <typename T, typename... U>
class Arg : public Arguments {
    // Holds function pointer to callback function
    T callback = nullptr;

    // Type of variable to be passed to callback function
    CB_Type cb_type;

    // Constants
    static const uint8_t MAX_ARG_LEN        =   8;  // Arg name length
    static const uint8_t MAX_HELP_LEN       =   50; // Help information length

    // Full name
    char name[MAX_ARG_LEN]{}; // Full name
    // Help information
    char help[MAX_HELP_LEN]{}; // Help information

public:

    // Command that has a callback function attached to it
    Arg(const char* _name, const char* _help, CB_Type _type,
        T cb = nullptr) : callback(cb) {
        if(!validate_arg(_name, _help)) {
            return;
        }
        cb_type = _type;
        strncpy(name, _name, MAX_ARG_LEN);
        strncpy(help, _help, MAX_HELP_LEN);
    }

    void call_callback(const char* msg) override {
        switch (cb_type){
            case _i16:
                callback(10);
                return;
            case _ch:
                callback(msg);
                return;
            default:
                Serial.println("error");
        }
    }
    const char* get_name() override {
        return name;
    };

    const char* get_help_info() override {
        return help;
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
