/**
 * Arduino Command Line Argument Parser (CLI for Arduino Ecosystem)
 * Copyright (C) 2023 Harvey Bates
 *
 * This program is free software: you can redistribute it and/or modify
 *         it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 *         but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef ARDUINO_CLAP_H
#define ARDUINO_CLAP_H

#include <Arduino.h>

//! Define to enable `range` and `loop` inbuilt functions
#define CLI_RANGE_LOOP

//! Message buffer for CLI commands entered by the user
static char cmd_buffer[100]{};

/**
 * @breif Generic CLI arguments class.
 *
 * Acts to enable the organisation of arguments that accept different types
 * (int, float, char etc.) into a single array that can easily be iterated
 * through. All functions are virtual and are overwritten by an `Argument` to
 * pass values around within the `ArduinoCLI` class.
 *
 * @note Documentation for these functions are provided in the `Argument` class.
 */
class Arguments {
public:
    virtual void execute_callback(const char* arg_val) = 0;
    virtual const char* get_name() = 0;
    virtual const char* get_help() = 0;
    virtual bool is_void_function() = 0;
};


/**
 * @brief Namespace to store conversions from const char* to various types.
 *
 * Supports:
 * - const char* (no-conversion, straight pass)
 * - float, double
 * - uint32_t, uint16_t, uint8_t
 * - int32_t, int16_t (int on UNO), uint8_t
 *
 * @note Overflow conversion revert to zero. For example, a conversion of
 * 40,000 to type int8_t will result in 0. The same goes for negative numbers
 * for unsigned ints and overflows.
 */
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


/**
 * @brief Command line argument.
 *
 * Stores the name of the argument, a help string and the callback the argument
 * should trigger. Two constructors are provided, one for callbacks that take
 * a value (of some type) and stand alone callbacks e.g. `callback(void)`.
 * Functions from within the Arguments class are overwritten here to provide
 * a passthrough between Arguments of different types.
 *
 * @warning The assigning of an argument can silently fail if the name or help
 * message are too long (15 and 75 characters respectively).
 *
 * @tparam T Type of argument expected in callback function.
 */
template <typename T>
class Argument : public Arguments {
    static const uint8_t MAX_ARG_LEN    =   15;    //! Argument name length
    static const uint8_t MAX_HELP_LEN   =   75;    //! Help information length
    char name[MAX_ARG_LEN]{};    //! Argument name
    char help[MAX_HELP_LEN]{};   //! Help information
    bool void_function = false;

public:
    //! Reference to a callback function that takes a value
    void(*callback)() = nullptr;
    void(*callback_t)(T) = nullptr;

    /**
     * @breif Several argument constructors are provided, accepting a various
     * number of values.
     *
     * @param _name Name of the argument that can be called from the cmd line.
     * @param _help Help message when the user types `help` in the cmd line.
     * @param cb Function that does not accept a value.
     */
    Argument(const char* _name, const char* _help, void(*cb)()) : callback(cb) {
        void_function = true;
        if(!build_arg(_name, _help)){
            return;
        }
    }

    //! An argument that accepts one value
    Argument(const char* _name, const char* _help, void(*cb)(T)) : callback_t(cb) {
        if(!build_arg(_name, _help)){
            return;
        }
    }

    bool build_arg(const char* _name, const char* _help){
        if(!validate_arg(_name, _help)) {
            return false;
        }
        strncpy(name, _name, MAX_ARG_LEN);
        strncpy(help, _help, MAX_HELP_LEN);
        return true;
    }

    /**
     * @brief Executes an arguments callback function.
     *
     * If the argument has a void callback function it will trigger the callback
     * without passing a value. If the callback function accepts a value the
     * value will be parsed from a const char* to whatever value the function
     * accepts.
     *
     * @param arg_val Argument value provided by user in CLI.
     */
    void execute_callback(const char* arg_val) override {
        // Callback with no value
        if(void_function){
            callback();
            return;
        }
        // Callback with a value type
        T v1 = ParseArg::type<T>(arg_val);
        callback_t(v1);
    }

    //! Get the name of the argument
    const char* get_name() override { return name; }
    //! Get help information for argument
    const char* get_help() override { return help; }
    //! Check if the function has value
    bool is_void_function() override { return void_function; }

private:
    /**
     * @brief Ensure command name and help information are valid.
     *
     * @param _name Argument name.
     * @param _help_info Argument help information.
     * @return Status of valid arguments (true if valid).
     */
    bool validate_arg(const char* _name, const char* _help_info){
        if(strlen(_name) > MAX_ARG_LEN || strlen(_help_info) > MAX_HELP_LEN){
            return false;
        }
        return true;
    }
};


/**
 * @brief Arduino command line interface that parses user input.
 * @note Maximum number of arguments is 10.
 */
class ArduinoCLI {
    //! Output stream (typically `Serial`)
    Stream& stream;
    //! Collection of command line arguments
    Arguments* args[10]{};
    //! Number of stored command line arguments
    uint8_t n_args = 0;
    //! Collection of command line argument sub arguments
    //Arguments* sub_args[5]{};
    ////! Number of stored command line sub arguments
    //uint8_t n_sub_args = 0;
    //! Buffer to hold a single line of a help message (see `help()`)
    char help_buffer[100]{};
#ifdef CLI_RANGE_LOOP
    //! Buffer to hold values that are passed to the inbuilt `array` function
    char* arr_buffer[20]{};
    //! Index within array buffer
    uint8_t arr_buffer_index = 0;
#endif

public:
    /**
     * @brief Constructor for ArduinoCLI.
     *
     * @param _serial Stream for incoming and outgoing msgs.
     */
    explicit ArduinoCLI(Stream& _serial) : stream(_serial) {}

    /**
     * @brief ArduinoCLI destructor, clears all arguments.
     */
    ~ArduinoCLI() {
        for(uint8_t i = 0; i < n_args; i++){
            free(args[i]);
        }
    }

    /**
     * @brief Add argument to CLI.
     *
     * @tparam T Dummy template (not used, or required by user).
     * @param name Name of argument.
     * @param help Help information surrounding argument.
     * @param cb Callback function that does not accept a value.
     */
    template <typename T = uint8_t>
    void add_argument(const char* name, const char* help,
                      void(*cb)()){
        args[n_args++] = new Argument<T>(name, help, cb); // No values (void)
    }

    template <typename T>
    void add_argument(const char* name, const char* help,
                      void(*cb)(T)){
        args[n_args++] = new Argument<T>(name, help, cb); // One value
    }

    /**
     * @brief CLI state for printing helpful error messages.
     */
    typedef enum {
        CLI_OK,
        CLI_ERROR,
        CLI_UNKNOWN_COMMAND,
        CLI_HELP_OK,
        CLI_EXPECTED_VALUE_NOT_FOUND,
    } CLI_Status;

private:
    /**
     * @brief Arduino CLI error handler based on `CLI_Status`.
     * @param input Input from user.
     * @param status Status of possible errors.
     */
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

    /**
     * @brief Prints out help information for all arguments and helper
     * functions.
     */
    void help(){
        stream.println("OPTIONS:");
        for(uint8_t i = 0; i < n_args; i++){
            print_help_line(args[i]->get_name(), args[i]->get_help());
        }
        stream.println("HELPERS:");
        print_help_line("help", "Print out help information.");
#ifdef CLI_RANGE_LOOP
        print_help_line("range", "Execute function with values within a range "
                                 "(start:stop:interval_ms).");
        print_help_line("loop", "Execute function in loop with values "
                                "(start:stop:interval_ms).");
        print_help_line("array", "Execute function with values provided in "
                                 "array (interval:[v1, v2...]).");
        print_help_line("stop", "Stop loop or array function.");
#endif // CLI_RANGE_LOOP
        print_help_line("exit", "Exit CLI cleanly.");
    }

    void print_help_line(const char* _name, const char* _help){
        snprintf(help_buffer, sizeof(help_buffer), "\t%-20s%-80s",
                 _name, _help);
        stream.println(help_buffer);
        memset(help_buffer, 0, sizeof(help_buffer));
    }

#ifdef CLI_RANGE_LOOP

    /**
     * @brief Parse message from `range` or `loop`.
     *
     * If the user supplies `range` or `loop` this function will extract
     * the values and carry out the `range()` or `loop()` function until
     * the user enters `stop` in the CLI.
     *
     * @param input Pointer to the argument provided by the user.
     * @param arg Reference to the argument that the `loop` or `range` will
     * be run on.
     * @return Status of the CLI.
     */
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
            execute_range_fn(arg, start, stop, interval);
        } else {
            execute_loop_fn(arg, start, stop, interval);
        }

        return CLI_OK;
    }

    /**
     * @brief Execute the `range` function.
     *
     * If the user provides the keyword `range` after an argument
     * that accepts an integer this function will parse out the supplied
     * range request (looks like this 0:10:250) where start:stop:interval.
     *
     * @param arg Argument to execute over range.
     * @param start Value at start of range.
     * @param stop Value at end of range.
     * @param interval Interval between start and stop increments.
     */
    void execute_range_fn(Arguments* arg, int32_t start, int32_t stop,
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

    /**
     * @brief Execute the `loop` function.
     *
     * The `loop` function increments between start and stop by 1 value every
     * interval (in ms). When reaching stop the `loop` function decrements by
     * 1 value every interval until it reaches the start value. It repeats this
     * until the user supplies the `stop` command in the cmd line.
     *
     * @param arg Argument to execute over range.
     * @param start Value at start of range.
     * @param stop Value at end of range.
     * @param interval Interval between start and stop increments.
     */
    void execute_loop_fn(Arguments* arg, int32_t start, int32_t stop,
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

    /**
     * @brief Strip non-digit character from a character array.
     *
     * An example being "[8" would be striped to "8". The negative symbol `-`
     * and the '.' character are ignored. For example, the `input` of "[-50.1]"
     * would result in "-50.1".
     *
     * @param input Character array to have its non-digit characters removed.
     * @return Character array with digits only (and `-` or `.`).
     */
    static char* strip_non_digits(char* input){
        char* dest = input;
        char* src = input;
        while(*src){
            if(isdigit(*src) || *src == '-' || *src == '.') {
                *dest++ = *src++;
            } else {
                src++;
            }
        }
        *dest = '\0';
        return input;
    }

    /**
     * @brief Parses the inbuilt `array` command into its tokens.
     *
     * The inbuilt `array` command allows the user to trigger a function with
     * a set of random values (provided by the user) in succession. The user
     * prefixes the random values with an interval which is used to break
     * up each command by that interval.
     *
     * @example
     * cmd-to-execute array interval:[v1, v2, v3 ...].
     *
     * @param input Command from user as parsed by the CLI.
     * @param arg Argument to execute using values in array and interval.
     * @return CLI status.
     */
    CLI_Status parse_array_cmd(char* input, Arguments* arg){
        if(!input){ return CLI_ERROR; }

        memset(arr_buffer, 0, sizeof(arr_buffer));
        arr_buffer_index = 0;

        // Extract interval between each array value
        input = strtok(nullptr, ":");

        if(!input){
            handle_error(input, CLI_EXPECTED_VALUE_NOT_FOUND);
            return CLI_EXPECTED_VALUE_NOT_FOUND;
        }

        uint32_t interval = ParseArg::type<uint32_t>(input);

        input = strtok(nullptr, ",");

        while(input){
            char* raw_value = strip_non_digits(input);
            arr_buffer[arr_buffer_index++] = raw_value;
            input = strtok(nullptr, ",");
        }

        execute_array_fn(arg, interval);

        return CLI_OK;
    }

    /**
     * @brief Executes inbuilt `array` command using values that were parsed in
     * the `parse_array_cmd()`.
     *
     * The user can type `stop` to halt the execution of this function at any
     * time.
     *
     * @param arg Argument with callback function to execute.
     * @param interval Interval between each callback function.
     */
    void execute_array_fn(Arguments* arg, uint32_t interval) {
        for(uint8_t i = 0; i < arr_buffer_index; i++){
            if(range_loop_exit()){ return; }
            arg->execute_callback(arr_buffer[i]);
            delay(interval);
        }
    }

    /**
     * @brief Checks if the user has supplied a `stop` command when in a
     * `loop` function.
     * @return Has the user entered stop?
     */
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

    static char* get_next_value(char* input){
        // Peek input to see if it is wrapped in quotations
        input = strtok(nullptr, "");
        if(!input){
            return input;
        }

        if(input[0] == '\"'){
            input = strtok(input, "\"");
        } else {
            input = strtok(input, " ");
        }

        return input;
    }

    /**
     * @brief Finds matches to user input and arguments and executes the
     * required functions based on the input provided.
     *
     * Once a command has been successfully extracted it is passed to this
     * function to carry out parsing and execution of various callback
     * functions based on the arguments name. Special arguments such as
     * `help`, `loop` and `range` are handled here as well.
     *
     * @param input User input from CLI.
     * @return Status of CLI.
     */
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
                // Void argument, trigger callback with no values
                if(args[i]->is_void_function()){
                    args[i]->execute_callback(input);
                    return CLI_OK;
                }

                // Get the arguments next value
                input = get_next_value(input);

                if(!input){
                    handle_error(input, CLI_EXPECTED_VALUE_NOT_FOUND);
                    return CLI_EXPECTED_VALUE_NOT_FOUND;
                }

                // Check to see if it is a special value
                #ifdef CLI_RANGE_LOOP
                if(strcmp("range", input) == 0 || strcmp("loop", input) == 0){
                    return parse_range_loop(input, args[i]);
                }
                if(strcmp("array", input) == 0){
                    return parse_array_cmd(input, args[i]);
                }
                #endif

                args[i]->execute_callback(input);
                return CLI_OK;
            }
        }

        handle_error(input, CLI_UNKNOWN_COMMAND);
        return CLI_UNKNOWN_COMMAND;
    }

    /**
     * @brief Extract a valid command from user input.
     *
     * Each valid argument is identified here. The search for valid commands
     * will continue until `input` equals a `nullptr` or a function does not
     * return `CLI_OK`.
     *
     * @param pCommand User input.
     * @return Should the CLI exit (only if `exit` is passed)
     */
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

    /**
     * @brief Exit the command line interface.
     * @param input Command from user.
     * @return True if CLI should exit.
     */
    bool exit(const char* input){
        if(strcmp("exit", input) == 0){
            stream.println("Exited command line.");
            return true;
        }
        return false;
    }

public:

    /**
     * @brief Main entrypoint for the CLI.
     *
     * Accepts and parses commands from the user. Commands will be parsed until
     * the user provides an `exit` command.
     */
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
