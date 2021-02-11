#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

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

// I will compile as one traslations unit.
// As such, I will use the internal keyword
// for all function definitions.
#ifndef internal
#define internal static
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

// I will need to hold the enums 
// within this stack, as such, 
// I will not create a general purpose 
// stack, and will focus on a super
// simple implementation.

#define SIZE 1024

typedef struct Stack 
{
    s32 values[SIZE];
    s32 index;
    s32 size;
} Stack; 

static Stack global_parser_stack;


internal void
stack_push(s32 value)
{
    Assert(global_parser_stack.size-1 < global_parser_stack.index);
    global_parser_stack.values[global_parser_stack.index++] = value;
}

internal b32 
stack_pop(s32 *value)
{
    if (global_parser_stack.index < 1) { return false; }
    *value = global_parser_stack.values[--global_parser_stack.index];
    return true;
}

internal b32 
stack_top(s32 *value)
{
    if (global_parser_stack.index < 1) { return false; }
    *value = global_parser_stack.values[global_parser_stack.index];
    return true;
}

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

/* ----- END Definitions ----- */ 

typedef struct String_Node
{
    char *text; 
    struct String_Node *next; 
} String_Node;

typedef struct String_List
{
    String_Node *head; 
    String_Node *tail; 
} String_List;

internal void
add_string_to_list(String_List *string_list, char *text)
{
    if (!string_list->head)
    {
        string_list->head = (String_Node *)malloc(sizeof(String_Node));
        string_list->head->next = NULL;
        string_list->tail = string_list->head;
        
    }
    if (!string_list->tail->next)
    {
        string_list->tail = string_list->tail->next;
        string_list->tail->text = text;
        string_list->tail->next = NULL;
    }
    else 
    {
        fprintf(stderr, "Error: string list has a next value when adding.");
    }
}

typedef struct Location
{
    s32 index;
    s32 line; 
    s32 character;
} Location; 

typedef enum Types
{
    Integer, 
    String, 
    Float, // will be implemented in the far future not for now. 
    TYPES_COUNT // this will be the count of all of the types as it will grow and shrink with the number of enum types.
} Types;

// Things for the future. 
typedef struct Ast_Node 
{
    char *key;  // has to be an string, if not then error it out. 
    void *value; // can be of many types, as such it is a pointer to memory.
    Types type;   // type of data contained in this.
    Location location;
    struct Ast_Node *next;
} Ast_Node; 

typedef struct Ast_Object
{
    String_List keys;
    Ast_Node *head_value;
    
    Ast_Node *values;
    
} Ast_Object; 

typedef struct Lexer
{
    char *text; // The buffer.
    Location location; 
    Ast_Node *node; // I think this will be the currently worked on node or something. 
} Lexer; 

enum Token_Types  
{
    TOKEN_LEFT_CURLY = 1,
    TOKEN_RIGHT_CURLY,
    TOKEN_SEMI_COLEN, 
    TOKEN_COLON,
    TOKEN_COMMA, 
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET, 
    TOKEN_VALUE
};

typedef struct Token
{
    Token_Types tk_type;
    Location location; 
    char *text; // if there is a key/value, the text will save it up for layter use.
} Token;

/**
 Checks if the character is a 'trash' value, and returns 
 a boolean value. True if it is, False if it isn't.
A 'trash' value is defined as a space, newline,
 tab, null terminator.
*/
internal b32
trash_value(char value)
{
    return value  == ' '|| value  == '\0' || 
        value == '\n'   || value == '\t';
}

internal b32
get_next_token(Lexer *lexer, Token *token_out)
{
    
    // TODO(ziv): Fix Location bugs.
    
    b32 success = false;
    
    if (!lexer->text[lexer->location.index]) { 
        return success; 
    }
    
    // Get rid of trash values like space, newline, tab, null terminator
    char current_char = *(lexer->text + lexer->location.index);
    while (trash_value(current_char))
    {
        if (current_char == '\n') // newline  
        {
            lexer->location.line++;
            lexer->location.character = 0;
        }
        lexer->location.character++;
        lexer->location.index++;
        current_char = *(lexer->text + lexer->location.index);
    }
    
    
    string slice_buffer = {0}; 
    slice_buffer.data = lexer->text + lexer->location.index;
    b32 is_value_token = false;
    
    s32 token_beginning_index = lexer->location.index; // I want the 
    // token location to be the beginning of its first character. 
    
    
#define TOKENIZE(token_character,token_type) if (*(lexer->text + lexer->location.index) == token_character) { \
lexer->location.index++; \
token_out->tk_type = token_type; \
token_out->text = (char *)token_character; \
success = true; \
}\
    
    
    TOKENIZE('{',TOKEN_LEFT_CURLY)
        else TOKENIZE('}', TOKEN_RIGHT_CURLY)
        else TOKENIZE('"', TOKEN_SEMI_COLEN) 
        else TOKENIZE(':', TOKEN_COLON)
        else TOKENIZE(',', TOKEN_COMMA) 
        else TOKENIZE('[', TOKEN_LEFT_BRACKET)
        else TOKENIZE(']', TOKEN_RIGHT_BRACKET)
        else
    {
        current_char = *(lexer->text + lexer->location.index);
        while ( !(trash_value(current_char) || 
                  current_char == '{' || current_char == '}' ||
                  current_char == ':' || current_char == ',' ||
                  current_char == '[' || current_char == ']' ||
                  current_char == '"') )
        {
            slice_buffer.size++;
            lexer->location.index++;
            current_char = *(lexer->text + lexer->location.index);
        }
        is_value_token = true;
        
    }
    if (is_value_token)
    {
        token_out->text = slice_to_string(slice_buffer); // TODO(ziv): implemented in a super hacky way, for debug purposes, should refactor this to be better in all ways. 
        token_out->tk_type = TOKEN_VALUE;
        success = true;
    }
    
    token_out->location = lexer->location; 
    lexer->location.character += (lexer->location.index - token_beginning_index); // advances
    // the lexer location character by the difference in the old and new indexes.
    
    return success;
}

/** Prints out a message that contains the token, where it was found, and the character/text that it contains. This is for debug purposes only, as it is kind of crappy and should most definetly change. 
*/ 
internal void
debug_print_token(Token token)
{
    
    char *token_type_in_string = ""; 
    switch (token.tk_type)
    {
        case TOKEN_LEFT_CURLY: 
        {
            token_type_in_string = "TOKEN_LEFT_CURLY";
        } break;
        
        case TOKEN_RIGHT_CURLY: 
        {
            token_type_in_string = "TOKEN_RIGHT_CURLY";
        } break;
        
        case TOKEN_SEMI_COLEN: 
        {
            token_type_in_string = "TOKEN_SEMI_COLEN";
        } break;
        case TOKEN_COLON: 
        {
            token_type_in_string = "TOKEN_COLON";
        } break;
        
        case TOKEN_COMMA: 
        {
            token_type_in_string = "TOKEN_COMMA";
        } break;
        
        case TOKEN_LEFT_BRACKET: 
        {
            token_type_in_string = "TOKEN_LEFT_BRACKET";
        } break;
        
        case TOKEN_RIGHT_BRACKET:
        {
            token_type_in_string = "TOKEN_RIGHT_BRACKET";
        } break;
        
        case TOKEN_VALUE:
        {
            token_type_in_string = "TOKEN_VALUE";
        } break;
        
    }
    
    char padding[255];
    s32 longest_size = string_size("TOKEN_RIGHT_BRACKET");
    s32 token_string_size = string_size(token_type_in_string);
    int counter = 0;
    for (; counter < longest_size - token_string_size; counter++)
    {
        padding[counter] = ' ';
    }
    padding[counter] = '\0';
    
    if (string_compare(token_type_in_string, "TOKEN_VALUE"))
    {
        printf("%s%s%d:%d  %s\n", token_type_in_string,padding, token.location.line, token.location.character,token.text); 
    }
    else
    {
        printf("%s%s%d:%d  %s\n", token_type_in_string, padding, token.location.line, 
               token.location.character, &((char)token.text)); // a hack because I can't bother with implementing a real solution to a 'non-problem'.
    }
    
}

void init_global_parser_stack()
{
#define STACK_SIZE 1024
    // NOTE: There is no need to be freed as it will get freed at 
    // the exit of the application.
    //global_parser_stack.values = (s32 *)malloc(STACK_SIZE); 
    global_parser_stack.size   = STACK_SIZE; 
    global_parser_stack.index  = 0; 
}

/* 
enum Parser_Types
{
    PARSER_OBJECT,
    PARSER_NEXT_KEY, 
    PARSER_KEY,
    PARSER_VALUE,
    PARSER_STRING
} Parser_Types; 
 */


typedef struct ParseStringInfo
{
    char *text; 
    b32 is_key; 
} StringType;

typedef struct Parser
{
    Ast_Object *head;
    Ast_Object *last_node;
    Ast_Object *current_node;
    
    ParseStringInfo string_info; 
} Parser;


internal Ast_Node *
parse_json(char *input_buffer)
{
    init_global_parser_stack();
    
    Lexer lexer = {0}; // Location will be initialized correctly.
    lexer.text = input_buffer; 
    lexer.location.line = 1; // Most code editors begin at line 1. 
    Token token = {0}; 
    
    b32 success = get_next_token(&lexer, &token);
    
    Parser parser = {0};
    
    while (success)
    {
        debug_print_token(token);
        
        s32 parser_scope; 
        // NOTE(ziv): When you don't use the 'parser_scope' value, and want it the next loop, you should push it back. 
        
        if (stack_pop(&parser_scope)) // the stack contains something.
        {
            
            if (parser_scope == TOKEN_LEFT_CURLY) // the beginning of an object.
            {
                parser.current_node = (Ast_Object *)malloc(sizeof(Ast_Object)); 
                parser.current_node->head_value = NULL; 
                parser.current_node->values = NULL; 
                stack_push(parser_scope);
            }
            else if (parser_scope == TOKEN_RIGHT_CURLY) // the end of an object.
            {
                // TODO(ziv): create the object and handle the child nodes and objects, that it might have already created but never connected to a head object.
                s32 last_scope; 
                stack_pop(&last_scope); 
                if (last_scope == TOKEN_LEFT_CURLY)
                {
                    
                }
            }
            else if (parser_scope == TOKEN_SEMI_COLEN) // beginning/end of string.
            {
                
                if (parser.string_info.is_key) // <- process key. 
                {
                    // TODO(ziv): Handle the creation of a key.
                    //Ast_Node *node = (Ast_Node *)malloc(sizeof(Ast_Node)); 
                    // create a key here. 
                }
                else if (!parser.string_info.is_key) // <- process value
                {
                    // TODO(ziv): Handle all of the things that you might need with, a value.
                    add_string_to_list(&parser.head->keys, parser.string_info.text);
                }
                
            }
            else if (parser_scope == TOKEN_VALUE) 
            {
                parser.string_info.text = token.text;
            }
            else if (parser_scope == TOKEN_COLON) // Marks the next string to be a value. 
            {
                if (parser.string_info.text)
                {
                    parser.string_info.is_key = false;
                }
                else
                {
                    // TODO(ziv): Think of a better error message. 
                    fprintf(stderr, "Error at %d:%d. Colon for a value before any key.", token.location.line, token.location.character);
                }
            }
            else if (parser_scope == TOKEN_COMMA)
            {
                // TODO(ziv): Implement this !!!
                stack_push(parser_scope); 
            }
            
        }
        else // The stack does not contain anything. I should just push the matching stuff.
        {
            // TODO(ziv): Continue implementing the many things that are missing here. 
            if (token.tk_type == TOKEN_VALUE)
            {
                // some value that needs construction. 
                fprintf(stderr, "Error at %d:%d. Begins with value.\n", token.location.line,token.location.character);
                return NULL;
            } 
            else if(token.tk_type == TOKEN_COMMA)
            {
                fprintf(stderr, "Error at %d:%d. Begins with comma.\n", token.location.line,token.location.character);
                return NULL;
            }
            else if(token.tk_type == TOKEN_RIGHT_CURLY)
            {
                fprintf(stderr, "Error at %d:%d. Begins with right curly bracket.\n", token.location.line,token.location.character);
                return NULL;
            }
            else if(token.tk_type == TOKEN_RIGHT_BRACKET)
            {
                fprintf(stderr, "Error at %d:%d. Begins with right bracket.\n", token.location.line,token.location.character);
                return NULL;
            }
            else
            {
                stack_push(token.tk_type);
            }
        }
        success = get_next_token(&lexer, &token);
    }
    
    if (!success) 
    {
        // we are on a bad time 
    }
    
    
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        char *file_name = argv[1]; 
        file_name = "C:\\dev\\json\\json_example.json"; 
        
        FILE *file = fopen(file_name, "r"); 
        if (file)
        {
            fseek(file, 0, SEEK_END); 
            size_t file_size = ftell(file); 
            char *buffer = (char *)malloc(file_size + 1); // will get freed when application ends.
            if (buffer)
            {
                fseek(file, 0, SEEK_SET);
                fread(buffer, file_size, 1, file); 
                buffer[file_size] = '\0';
                
                
                fprintf(stdout, "Json data:\n%s\n\n", buffer);
                
                fclose(file);
                
                // 
                // Begin the parsing  
                //
                
                
                Ast_Node *ast_tree = parse_json(buffer); 
                // This node is the AST tree head node. 
            }
            else
            {
                fprintf(stderr, "Error: Cannot allocate space for file buffer.\n"); 
            }
        }
        else
        {
            fprintf(stderr, "Error: Can not open file %s.\n", file_name); 
            
        }
        
    }
    else
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]); 
    }
    return 0;
}


/* --- Beginners thoughts on parsing and lexing --- */

// My first 'solution' is to go and lex until you reach 
// the lowest scope in the region that you are in. 
// This region will contain hopfully some primitive 
// type that will nicely be put into a AST like node. 
// To know which scope you are at currently you pop 
// the scope from the scope type stack ( type enum ). 
// At the beginning of the scope you push into the 
// stack, and if you reach the end of the scope 
// you will pop and check if you have reached the 
// end. 

// The second 'solution' is more of what I saw being 
// on a real world compiler of a language called 'sif'.
// There there was a lexer struct being passed around, 
// there all of the needed context information is 
// passed around nicely within all of the funcitons. 
// other then that there are not a ton of things that 
// need to change from my first solution.

/* ---- After lexing, and beginning of parsing ---- */

// My first thoughts were kind of weird. I thought 
// that the lexer was a sepret part frome the parser, 
// and you parse first and only then lexe and then 
// you bulid the AST. 
// As I worked on this project I was proven wrong. 
// The whole process is parsing while some of it 
// is lexing. 

// --- Lexing ---- 
// Lexing is easy, you just tokenize all of the 
// characters as needed and pass it to the parser. 
// Plus the tokens should contain their location 
// in the file for more debug and error information. 

// --- Parsing --- 
// This is the harder part, as it is not as intuitive. 
// I have yet to finish this part and thinking about 
// how to represent a structure that contains a pointer
// to a sturcture is not easy. This also means that I 
// currently don't have any way of doing so, and I will 
// move twards that goal (I have an idea alread) in the 
// future.

/* ------------------------------------------------ */




// NOTE(ziv):
// The following  is not needed for the parser.
// It was done to get argumens for the command line and 
// do something with them. Cleanup is needed.
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