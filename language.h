/* date = February 12th 2021 7:10 pm */

#ifndef LANGUAGE_H
#define LANGUAGE_H

/* --- Type defenitions for project --- */ 
#include <stdint.h>
typedef int8_t s8; 
typedef int16_t s16; 
typedef int32_t s32; 
typedef int64_t s64;

typedef uint8_t  u8; 
typedef uint16_t u16; 
typedef uint32_t u32; 
typedef uint64_t u64;

typedef int    b32; 
typedef float  f32; 
typedef double f64;

#define false 0
#define true 1

#define DUBUG

#if DEBUG
#define Assert(expression) if(!(expression)) { *(int *)0 = 0; } 
#else
#define Assert(expression)
#endif

// This assumes that if you use the unity compilation model
// you use the internal keyword. If not, you can thank me later :)
// This is done this way so if you don't use the internal 
// keyword it won't affect you really, but it you do, it will 
// bring you benefits.
#ifndef internal
#define internal
#endif

#define size_t s64

// My way of handling error that I like to do sometimes.
// That does mean that it should be used carefuly(maybe).
typedef enum Error_Messages
{
    Success,
    Failure,
} Error_Messages;

#define CreateTypeAndErrorType(type) \
typedef struct type_and_error        \
{                                    \
type result;                         \
Error_Messages error;                \
} Type_And_Error                     \

#define WrapError(type) type_and_error

// create a new type
CreateTypeAndErrorType(int);

/* --- Stack definitions: --- */

/* --- String definitions: --- */

// This is the definiiton of a slice
typedef union string_slice
{
    struct 
    {
        char *data;
        s32 size;
    };
    
    struct
    {
        char *data; 
        s32 index;
    };
} string, string_slice;


typedef struct String_Buffer
{
    char *data; 
    int   size; 
    int   index;
} String_Buffer;


// TODO(ziv): REDO THIS PLEASE !!! I don't like how it is currently, and so, it should be refactored.

internal void
default_string_to_slice(char *buffer, string slice)
{
    slice.size = 0;
    slice.data = buffer; 
    
    for (; *buffer; buffer++)
    {
        slice.size++; 
    }
}

/** computes the size of a given string that has a null terminator. */
internal int 
string_size(char *string)
{
    int string_size = 0; 
    while (*string)
    {
        string_size++;
        string++;
    }
    return string_size;
}

internal b32 
string_compare(char *value1, char *value2)
{
    b32 success = true;
    for (; *value1; value1++)
    {
        if (*value2++ != *value1)
        {
            success = false;
        }
    }
    return success;
}

internal char *
slice_to_string(string slice)
{
    
    char *result = (char *)malloc(slice.size+1); 
    char *temp_char = result; 
    for (s32 counter = 0; counter < slice.size; counter++)
    {
        *temp_char++ = *slice.data++;
    }
    *temp_char = '\0';
    return result;
}


// extracts a word upto a whitespace newline or a null terminator.
// the word is returned and the index in the command line is updated. 
internal char *
extract_word_no_white_space(String_Buffer *command_line, String_Buffer *buffer)
{
    char *cmd_data      = command_line->data;
    int  data_index     = command_line->index;
    int  starting_index = data_index;
    
    char *result        = buffer->data + buffer->index; 
    // if the first characters is a whitespace it is not a 'word'.
    if (*cmd_data == ' ') { return NULL; } 
    
    char *temp_result   = result;
    char *current_char  = cmd_data + data_index;
    
    do {
        
        *temp_result++ = *current_char;
        data_index++;
        current_char = cmd_data + data_index;
        
    } while (data_index < command_line->size && *current_char != ' ' && *current_char != '\n');
    
    // eat all whitespace after the 'word'. 
    for ( ; *current_char && *current_char == ' '; current_char = cmd_data + data_index)
    {
        data_index++;
    }
    
    command_line->index = data_index;
    
    *temp_result++ = '\0'; 
    
    buffer->index += data_index - starting_index;
    return result;
}

// converts a character of a number between 0 and 9
// into a integer eqivelent.
internal int 
convert_char_to_positive_int(char character)
{
    int result = character - '0'; 
    if ( 0 <= result && result <= 9)
    {
        return result;
    }
    return -1;
}

// converts a string given into a 'positive' integer. 
// if the string given is not a number or not an integer
// will return an error code -1.
internal int 
convert_string_to_positive_int(char *string, int size)
{
    // simple conversion where theres is not much to do.
    
    int result  = 0;
    int exp     = 1;
    int counter = 0;
    
    while (counter < size-1) 
    {
        counter++; 
        exp *= 10; 
    }
    
    do {
        
        int next_number = convert_char_to_positive_int(*string);
        if ( next_number == -1 ) { return -1 ; }
        result += next_number * exp;
        exp /= 10;
        string++;
        
    } while ( *string );
    
    return result;
}

// Converts a 'char *' string to a integer.
internal WrapError(int) 
convert_string_to_integer(char *string, int size)
{
    WrapError(int) result = {0}; // error is failer
    
    b32 is_negative = *string == '-';
    if (is_negative) 
    {
        string++; // ignore the '-'
    }
    
    int positive_int = convert_string_to_positive_int(string, string_size(string)); 
    if ( positive_int == -1 ) { return result; }
    result.result = positive_int * is_negative * -1 + positive_int * !is_negative;
    result.error  = Success; 
    
    return result; 
}

#endif //LANGUAGE_H
