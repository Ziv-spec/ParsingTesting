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

#define false 0
#define true 1

// I will compile as one traslations unit.
// As such, I will use the internal keyword
// for all function definitions.
#ifndef internal
#define internal static
#endif

#define size_t s64


// My way of handling error that I like to do sometimes. 
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


// Takes a slice and copise it's data int a default string.
// NOTE(ziv): This function does not take the 'buffer' length. 
// As such, there is no way to tell if this funciton will 
// overrun the 'buffer'. This can create a problem and should 
// not be used for the most part (slices don't have this problem).
internal void 
slice_to_default_string(string slice, char *buffer) 
{
    int slice_index = slice.index;
    while (slice.size < slice_index) 
    {
        *(buffer + slice_index) = *(slice.data + slice_index); 
    }
}


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

// This is kind of the things for the future. 
typedef struct Ast_Node 
{
    string key;  // has to be an string, if not then error it out. 
    void *value; // can be of many types, as such it is a pointer to memory.
    Types type;   // type of data contained in this.
    Location location;
    struct Ast_Node *next;
} Ast_Node; 

typedef struct Ast_Object
{
    string *keys;
    Ast_Node *values;
    Types type;
} Ast_Object; 

typedef struct Lexer
{
    char *text; // The buffer.
    Location location; 
    Ast_Node *node; // I think this will be the currently worked on node or something. 
} Lexer; 

enum Token_Types  
{
    TOKEN_LEFT_CURLY,
    TOKEN_RIGHT_CURLY,
    TOKEN_SEMI_COLEN, 
    TOKEN_COLON,
    TOKEN_COMMA, 
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET, 
    TOKEN_STRING
};

typedef struct Token
{
    Token_Types tk_type;
    Location location; 
    char *text; // if there is a key/value, the text will save it up for layter use.
} Token;

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

internal b32
trash_value(char value)
{
    return value  == ' '|| value  == '\0' || 
        value == '\n'   || value == '\t';
}


#define TOKENIZE(token_character,token_type) if (*(lexer->text + lexer->location.index) == token_character) { \
lexer->location.index++; \
token_out->tk_type = token_type; \
token_out->text = (char *)token_character; \
success = true; \
}\

internal b32
get_next_token(Lexer *lexer, Token *token_out)
{
    
    // tokenize
    b32 success = false;
    
    if (!lexer->text[lexer->location.index])
    {
        fprintf(stderr, "Error: end of file while parsing.");
        return success;
    }
    
    
    char current_char = *(lexer->text + lexer->location.index);
    while (trash_value(current_char))
    {
        if (current_char == '\n') 
        {
            lexer->location.line++;
        }
        lexer->location.character++;
        lexer->location.index++;
        current_char = *(lexer->text + lexer->location.index);
    }
    
    
    string slice_buffer = {0}; 
    slice_buffer.data = lexer->text + lexer->location.index;
    b32 is_value_token = false;
    
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
        token_out->text = slice_to_string(slice_buffer);
        token_out->tk_type = TOKEN_STRING;
        success = true;
    }
    
    token_out->location = lexer->location;
    return success;
}

/* 
internal void 
parse_next()
{
    
}
 */

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

internal void
print_token(Token token)
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
        
        case TOKEN_STRING:
        {
            token_type_in_string = "TOKEN_STRING";
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
    
    if (string_compare(token_type_in_string, "TOKEN_STRING"))
    {
        
        printf("_%s_ %s%s at %d:%d\n", token_type_in_string, padding, token.text, token.location.line, token.location.character); 
        
    }
    else
    {
        printf("_%s_ %s%s at %d:%d\n", token_type_in_string,padding, &((char)token.text), token.location.line, token.location.character); 
    }
    
}

internal Ast_Node *
parse(char *input_buffer)
{
    
    Lexer lexer = {0};
    lexer.text = input_buffer; 
    lexer.location; 
    
    Token token = {0}; 
    
    b32 success = get_next_token(&lexer, &token);
    do 
    {
        if (success) 
        {
            print_token(token);
        }
        success = get_next_token(&lexer, &token);
    } while (success);
    
    return NULL;
}

//fprintf(stderr, "Error: Size not matching (%d != %d). Line: %d\n", input_buffer.size, output_buffer.size, __LINE__);

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        char *file_name = argv[1]; 
        file_name = "../json_example.json"; 
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
                
                fprintf(stdout, "%s\n\n", buffer);
                
                fclose(file);
                
                // 
                // Begin the parsing  
                //
                
                
                Ast_Node *ast_tree = parse(buffer); 
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

/* ------------------------------------------------ */

/* ---    AST Like Structure for json format    --- */ 

// The struct will probably look something like this: 

// first attempt at thinking of something *NOTE* will not really work, 
// I know this as I have though this through and there are some cases 
// where it does not handle, I should have one more type. 

#if 0
typedef struct Location // mainly done for extra information for debugging and knowledge of location when parsing.
{
    s32 row;    // line 
    s32 column; // column ...
    
    // the location of the thing in the file or something like that for the mostaprt this is just to track the thing for errosr I htink I will think about htis more deeply in some time. 
} Location; 

typedef struct Ast_Node
{
    string key;  // has to be an string, if not then error it out. 
    void *value; // can be of many types, as such it is a pointer to memory.
    Type type;   // type of data contained in this.
    Location location;
    struct Ast_Node *next;
} Ast_Node; 

typedef struct Ast_Block
{
    Ast_Node *next_node; 
    Type type;
    Location location;
} Ast_Block; 
#endif

// thet hign htat will happen is I will have 2 ast _ nodes and then I will forge a block that will be its parent 
// where as the nodes will connect to each other, and there will not be any need for anything else.
// this should work though there will not be any reas reason for the count then. 
// I need to think of key-value pairs.
