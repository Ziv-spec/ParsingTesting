#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "language.h"
#include "lexer.h"

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

void init_global_parser_stack()
{
#define STACK_SIZE 1024
    // NOTE: There is no need to be freed as it will get freed at 
    // the exit of the application.
    //global_parser_stack.values = (s32 *)malloc(STACK_SIZE); 
    global_parser_stack.size   = STACK_SIZE; 
    global_parser_stack.index  = 0; 
}

typedef struct String_Node
{
    char *text; 
    struct String_Node *next; 
} String_Node;

typedef struct String_List
{
    String_Node *head; 
    String_Node *tail; 
    s32 string_list_size;
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

typedef struct Value_Node
{
    void *value; 
    enum Node_Type type;
    struct Value_Node *next; 
} Value_Node;

typedef struct Value_List
{
    Value_Node *head; 
    Value_Node *tail; 
    // The size params are meant to check if the parser is doing thing correctly and doesn't mess up the size of each list.
    s32 value_list_size;
} Value_List;

internal void
add_value_to_list(Value_List *value_list, void *value, s32 type)
{
    if (!value_list->head) 
    {
        value_list->head = (Value_Node *)malloc(sizeof(Value_Node));
        value_list->head->next = NULL;
        value_list->tail = value_list->head;
        
    }
    if (!value_list->tail->next)
    {
        value_list->tail = value_list->tail->next;
        value_list->tail->value = value;
        value_list->tail->next = NULL;
    }
    else 
    {
        fprintf(stderr, "Error: string list has a next value when adding.");
    }
}

typedef enum Node_Type
{
    TYPE_STRING,
    TYPE_INTEGER, 
    TYPE_FLOAT, // will be implemented in the far future not for now. 
} Types;

typedef struct Ast_Node 
{
    String_List keys;
    Value_List values;
    struct Ast_Node *next;
} Ast_Node; 

typedef struct ParserStringInfo
{
    char *text; 
    b32 handle_key; 
    b32 handle_value;
} StringType;

typedef struct Parser
{
    //Ast_Object *object;
    ParserStringInfo string_info; 
} Parser;

internal Ast_Node *
parse_json(char *input_buffer)
{
    init_global_parser_stack();
    init_token_to_string_map();
    
    Lexer lexer = {0}; // Location will be initialized correctly.
    lexer.text = input_buffer; 
    lexer.location.line = 1; // Most code editors begin at line 1. 
    
    Token token = {0}; 
    
    Parser parser = {0};
    //parser.string_info.handle_key = 1;
    
    b32 error = get_next_token(&lexer, &token);
    while (!error)
    {
        debug_print_token(token);
        
#if 0
        s32 parser_scope; 
        // NOTE(ziv): When you don't use the 'parser_scope' value, and want it the next loop, you should push it back. 
        
        if (stack_pop(&parser_scope)) // the stack contains something.
        {
            
            if (parser_scope == TOKEN_LEFT_CURLY) // the beginning of an object.
            {
                /* 
                    parser.current_node = (Ast_Object *)malloc(sizeof(Ast_Object)); 
                    parser.current_node->head_value = NULL; 
                    parser.current_node->values = NULL; 
     */
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
                if (!parser.string_info.handle_value && !parser.string_info.text) 
                {
                    // TODO(ziv): Handle all of the things that you might need with, a value.
                    parser.string_info.handle_key = true;
                    
                }
                else if (parser.string_info.handle_key && parser.string_info.text)
                {
                    // TODO(ziv): Handle the creation of a key.
                    //add_string_to_list(&parser.head->keys, parser.string_info.text);
                }
                else if (parser.string_info.handle_value && parser.string_info.text)
                {
                    Ast_Node *value_node = (Ast_Node *)malloc(sizeof(Ast_Node)); 
                    //value_node->key = parser.string_info.text; 
                    value_node->next = NULL;
                    
                    
                    // link agains the current node. 
                    
                    
                    //value_node->value = ;
                    
                    
                    // not here, put it in the colon part. 
                    char *text = parser.string_info.text;
                    WrapError(int) integer = convert_string_to_integer(text, string_size(text));
                    if (!integer.error)
                    {
                        
                    }
                    else 
                    {
                        
                        fprintf(stderr, "Error: invalid argument. Expected integer.\n");
                    }
                    
                    
                    
                    //value_node->type = TYPE_STRING;
                    
                }
                
            }
            else if (parser_scope == TOKEN_VALUE) 
            {
                parser.string_info.text = token.text;
            }
            else if (parser_scope == TOKEN_COLON) // Marks the next string to be a value. 
            {
                if (parser.string_info.handle_key && parser.string_info.text) // key exists
                {
                    parser.string_info.handle_value = true;
                }
                else // there was no key, and the colon means a value.
                {
                    // TODO(ziv): Think of a better error message. 
                    fprintf(stderr, "Error at %d:%d. Colon for a value before any key.\n", token.location.line, token.location.character);
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
                fprintf(stderr, "Error at %d:%d. Begins with right curly bracket. Should begin with a left one.\n", token.location.line,token.location.character);
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
#endif
#define DEBUG 1
#if DEBUG
        printf(" %d %s\n", error, token.text);
#endif
        error = get_next_token(&lexer, &token);
        
    }
    
    if (error) 
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