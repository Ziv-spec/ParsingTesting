#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

/* Type defenitions for project */ 
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

// I will compile as one traslations unit.
// As such, I will use the internal keyword
// for all function definitions.
#ifndef internal
#define internal static
#endif

#define size_t s64

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


// --- String definitions: --- 

typedef struct String_Buffer
{
    char *data; 
    int   size; 
    int   index;
} String_Buffer;

// computes the size of a given string that has a null terminator.
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

// NOTE(ziv): Deprecated!!! if you need to use this you are probably doing something wrong!!!
// returns the next word without updating the index. 
// This will result in the ability to look up words
// more than once.
internal char *
look_up_next_word(String_Buffer *command_line, String_Buffer *buffer)
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
    
    *temp_result++ = '\0'; 
    
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

internal void 
parse_command_line(char *command_line, int size)
{
    if (command_line[size-1] == '\n') { command_line[size-1] = '\0'; } // I don't like newline at the end of the 'tget()' as it is bad for printf to the console for this specific use case. Thought the size still is the same so I think it should not effect much the things that happen. 
    
    String_Buffer command_line_arg = { 0 }; 
    command_line_arg.data = (char *)command_line; 
    command_line_arg.size = size;
    
    String_Buffer buffer = { 0 }; 
    buffer.data  = (char *)malloc(sizeof(char) * size);
    buffer.size = size;
    
    char *command = extract_word_no_white_space(&command_line_arg, &buffer);
    if (strcmp(command, "print") == 0) 
    {
        
        /*         
                // TODO(ziv): look into how to handle this case in a more elegane and robust way.
                
                char *word2 = extract_word_no_white_space(&command_line_arg, &buffer);
                if (!*word2) { goto free_mem; } // end of the command_line, no need to continue. 
                WrapError(int) first_number = convert_string_to_integer(*word2);
                if (first_number.error) { goto free_mem }
                // a check needed for the length of the string so that it will be known the range 
                // of the inserted thing.
                printf("'%d', ", first_number.result);
                
                char *word3 = extract_word_no_white_space(&command_line_arg, &buffer);
                if (!*word3) { goto free_mem; } // end of the command_line, no need to continue. 
                printf("'%s'\n", word3);
                 */
        
    }
    else if (strcmp(command, "print_two") == 0) 
    {
        char *word1 = extract_word_no_white_space(&command_line_arg, &buffer);
        if (!*word1) { goto free_mem; } // end of the command_line, no need to continue. 
        WrapError(int) first_number = convert_string_to_integer(word1, string_size(word1));
        if (first_number.error) { goto free_mem; }
        
        char *word2 = extract_word_no_white_space(&command_line_arg, &buffer);
        if (!*word2) { goto free_mem; } // end of the command_line, no need to continue.
        int word2_size = string_size(word2);
        int second_number = convert_string_to_positive_int(word2, word2_size);
        if ( second_number == -1 ) { return; } // error while converting into an int. 
        
        printf("first number %d\n", first_number.result);
        printf("The second number is : %d\n", second_number);
    }
    
    
    
    // free all of the memory at the end.
    free_mem:
    free(buffer.data);
    return;
}




#define MAX 1000
int main()
{
    
    char read_line[MAX]; 
    
    while (1)
    {
        
        printf("> ");
        fgets(read_line, MAX, stdin); 
        
        if (strcmp(read_line, "exit\n") == 0)
        {
            break;
        }
        
        if (strcmp(read_line, "exit \n") == 0)
        {
            break;
        }
        
        if (!*read_line) 
        {
            break;
        }
        
        int size = string_size(read_line);
        parse_command_line(read_line, size); 
    }
    
    return 0;
}







/* The syntax should be something like this: */

/* NOTE(ziv):
char *word = extract_word_no_white_space(&command_line_arg, &string_buffer); 
if ( !*word ) { deallocate memory and exit } 
int extracted_number = convert_string_to_positive_int(word, string_size(word)); 
if ( extracted_number == -1 ) { deallocated memory and exit}
// Use the extracted number from the word


// ----------------------

One more option would be something like the following: 

// for positive numbers only
int extracted_number =  extract_positive_number(&command_line_arg, &string_buffer); 
if (extracted_number == -1) { deallocate memory and exit } 

// for all integer numbers 

char *word = extract_word_no_white_space(&command_line_arg, &string_buffer); 
if (!*word) { deallocate memory and exit } 
if (*word == '-')
{
 *   word++; 
*   int extracted_number = convert_string_to_positive_int(word, string_size(word));
*   if ( extracted_number == -1 ) { deallocated memory and exit}
*   extracted_number *= -1;
*   // do something with the number 
}
else
{
*   int extracted_number =  extract_positive_number(&command_line_arg, &string_buffer); 
*   if (extracted_number == -1) { deallocate memory and exit } 
}
 

TODO(ziv): have error messages in the parser so that I will know what the parser 
thinks is wrong in the format, and you I will be able to check that.
*/


/* 
Now I would like to parser json file and print it correctly. 
this means that I will put the json file in a tree like 
structure where I will have to create a new printing like 
something that will print it, new enum types to know which 
type to print/cast and have just the just working and doing 
it's job. 

All of the json syntax: 

{}   <- object 
[]   <- list 
""   <- string key
:    <- value will come next.
,    <- next value

""          <- string value
 null        <- NULL value
 0123456789  <- int value
0.123456789 <- float value 
 
NOTES: keys must be "strings". 

So, the first project would be doing something simple, and the progresively be more 
and more complex while doing these types of stuff whlie the end goal is to hav a 
josn file that after parsed will create a C struct that will be able to contain the 
data natively. 

*/
