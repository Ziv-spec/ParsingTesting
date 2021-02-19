#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 1
#define internal static
#include "language.h"


typedef struct Location
{
    s32 index;
    s32 line; 
    s32 character;
} Location; 

typedef struct Lexer
{
    char *text; // The buffer.
    Location location; 
    //Ast_Node *node; // I think this will be the currently worked on node or something. 
} Lexer; 

enum Token_Types  
{
    TOKEN_LEFT_CURLY = 1,
    TOKEN_RIGHT_CURLY,
    TOKEN_RIGHT_BRACKET, 
    TOKEN_LEFT_BRACKET,
    TOKEN_SEMI_COLEN, 
    TOKEN_COLON,
    TOKEN_COMMA, 
    TOKEN_INTEGER,
    TOKEN_STRING, 
    TOKEN_FLOAT,
    TOKEN_COUNT
};

#if DEBUG
static char *token_name_map[TOKEN_COUNT]; 

void init_token_to_string_map()
{
    token_name_map[TOKEN_LEFT_CURLY]    = "TOKEN_LEFT_CURLY"; 
    token_name_map[TOKEN_RIGHT_CURLY]   = "TOKEN_RIGHT_CURLY"; 
    token_name_map[TOKEN_RIGHT_BRACKET] = "TOKEN_RIGHT_BRACKET"; 
    token_name_map[TOKEN_LEFT_BRACKET]  = "TOKEN_LEFT_BRACKET"; 
    token_name_map[TOKEN_SEMI_COLEN]    = "TOKEN_SEMI_COLEN";
    token_name_map[TOKEN_COLON]         = "TOKEN_COLON";
    token_name_map[TOKEN_COMMA]         = "TOKEN_COMMA"; 
    
    token_name_map[TOKEN_INTEGER]       = "TOKEN_INTEGER"; 
    token_name_map[TOKEN_STRING]        = "TOKEN_STRING"; 
    token_name_map[TOKEN_FLOAT]         = "TOKEN_FLOAT"; 
}
#endif

typedef struct Token
{
    Token_Types tk_type;
    Location location; 
    char *text; // if there is a key/value, the text will save it up for layter use.
} Token;

/** Prints out a message that contains the token, where it was found, and the character/text that it contains. This is for debug purposes only, as it is kind of crappy and should most definetly change. 
*/ 
internal void
debug_print_token(Token token)
{
    
    char *token_name = token_name_map[token.tk_type];
    
    char padding[255];
    s32 longest_size = string_size("TOKEN_RIGHT_BRACKET")-1;
    s32 token_string_size = string_size(token_name);
    int counter = 0;
    for (; counter < longest_size - token_string_size; counter++)
    {
        padding[counter] = ' ';
    }
    padding[counter] = '\0';
    
    printf("%s%s%d:%d  %s\n", token_name,padding, token.location.line, token.location.character, token.text); 
}

enum Lexer_Error
{
    TOKEN_MESSAGE_SUCCESS = 0, // should be zero so 'if (!error)' would be possible.
    TOKEN_MESSAGE_STRING  = 1,
    TOKEN_MESSAGE_INTEGER = 2, 
    TOKEN_MESSAGE_FLOAT   = 3,
    TOKEN_MESSAGE_END     = 4,  
} Lexer_Error_Code; 

#define is_number(value) ('0' <= value && value <= '9')

/**
Checks if the character is a 'trash' value, and returns 
a boolean value. True if it is, False if it isn't.
A 'trash' value is defined as a space, newline,
tab, null terminator.
*/
inline b32
trash_value(char value)
{
    return value  == ' '|| value == '\n'   || value == '\t';
}

internal b32
get_next_token(Lexer *lexer, Token *token_out)
{
    
    Lexer_Error error = TOKEN_MESSAGE_END;
    if (!lexer->text[lexer->location.index]) { 
        return error; 
    }
    
    // Get rid of trash values like space, newline, tab, null terminator
    char cursor = *(lexer->text + lexer->location.index); // cursor that points to the latest character.
    while (trash_value(cursor))
    {
        if (cursor == '\n')
        {
            lexer->location.line++;
            lexer->location.character = 0;
        }
        lexer->location.character++;
        lexer->location.index++;
        cursor = *(lexer->text + lexer->location.index);
    }
    
    s32 token_beginning_index = lexer->location.index; // For error messages,
    // I make the problem with the string show up as it's first character 
    //and not the last. For that reason I do this. 
    
    // NOTE(ziv): This should ONLY be used with a *string* in the first argument, and a *token type* in the second argument.
#define TOKENIZE(token_string,token_type) if (*(lexer->text + lexer->location.index) == *token_string) { \
lexer->location.index++;                   \
token_out->tk_type = token_type;           \
token_out->text = token_string;            \
error = TOKEN_MESSAGE_SUCCESS;             \
}                                          \
    
    TOKENIZE("{",TOKEN_LEFT_CURLY)
        else TOKENIZE("}", TOKEN_RIGHT_CURLY)
        else TOKENIZE(":", TOKEN_COLON)
        else TOKENIZE(",", TOKEN_COMMA) 
        else TOKENIZE("[", TOKEN_LEFT_BRACKET)
        else TOKENIZE("]", TOKEN_RIGHT_BRACKET)
        else 
    {
        string slice_buffer = {0}; 
        
        if (cursor == '"') // tokenize string
        {
            lexer->location.index++; // skip the " 
            slice_buffer.data = lexer->text + lexer->location.index; // beginning of slice.
            cursor = *(lexer->text + lexer->location.index);
            for (;cursor && cursor != '"'; cursor = *(lexer->text + lexer->location.index))
            {
                lexer->location.index++;
                slice_buffer.size++;
            }
            lexer->location.index++; // skip the "
            if (cursor)
            {
                char *token_text = (char *)malloc(slice_buffer.size + 1);
                int counter = 0;
                for (; counter < slice_buffer.size; counter++)
                {
                    token_text[counter] = slice_buffer.data[counter];
                }
                *(token_text + counter) = '\0';
                token_out->text = token_text;
                token_out->tk_type = TOKEN_STRING;
                error = TOKEN_MESSAGE_SUCCESS;
            }
            else
            {
                error = TOKEN_MESSAGE_STRING;
            }
            
        }
        else // tokenize integer/float
        {
            slice_buffer.data = lexer->text + lexer->location.index; // beginning of slice.
            b32 is_floating_point = false;
            cursor = *(lexer->text + lexer->location.index);
            for (; is_number(cursor); cursor = *(lexer->text + lexer->location.index))
            {
                lexer->location.index++;
                slice_buffer.size++;
            }
            
            // TODO(ziv): Fix the case: '0.' . In that case, infinite loop.
            // Now that I have checked another time, it does not seem like a problem. 
            // will have to do more in depth check to know for sure.
            if (cursor == '.')
            {
                is_floating_point = true;
                
                lexer->location.index++; // skip the '.'
                slice_buffer.size++;
                cursor = *(lexer->text + lexer->location.index);
                for (;'0' < cursor && cursor < '9'; cursor = *(lexer->text + lexer->location.index))
                {
                    lexer->location.index++;
                    slice_buffer.size++;
                }
            }
            else
            {
                cursor = *(lexer->text + lexer->location.index-1);
                if (!is_number(cursor))
                {
                    error = TOKEN_MESSAGE_INTEGER;
                }
            }
            
            char *token_text = (char *)malloc(slice_buffer.size + 1);
            int counter = 0;
            for (; counter < slice_buffer.size; counter++)
            {
                token_text[counter] = slice_buffer.data[counter];
            }
            *(token_text + counter) = '\0';
            token_out->text = token_text;
            error = TOKEN_MESSAGE_SUCCESS;
            if (is_floating_point)
            {
                token_out->tk_type = TOKEN_FLOAT;
            }
            else
            {
                token_out->tk_type = TOKEN_INTEGER;
            }
        }
        
    }
    
    // synces the out token location.  
    token_out->location = lexer->location; 
    lexer->location.character += (lexer->location.index - token_beginning_index);
    
    return error;
}

internal b32
peek_next_token(Lexer *lexer, Token *token)
{
    Location old_location = lexer->location;
    b32 error = get_next_token(lexer, token); 
    lexer->location = old_location;
    return error;
}

typedef enum Value_Type
{
    VALUE_TYPE_NONE,
    VALUE_TYPE_OBJECT,
    VALUE_TYPE_LIST,
    VALUE_TYPE_STRING,
    VALUE_TYPE_INTEGER, 
    VALUE_TYPE_FLOAT
} Value_Type;

typedef struct Key_Value_Node
{
    char *key; 
    void *value;
    Value_Type value_type; 
    struct Key_Value_Node *next;
} Key_Value_Node;

typedef struct Key_Value_List
{
    Key_Value_Node *head; 
    Key_Value_Node *tail;
} Key_Value_List;

internal Key_Value_Node *
init_key_value_node()
{
    Key_Value_Node *key_value_map = (Key_Value_Node *)malloc(sizeof(Key_Value_Node)); 
    key_value_map->key = NULL; 
    key_value_map->value = NULL; 
    key_value_map->value_type = VALUE_TYPE_NONE; 
    key_value_map->next = NULL;
    return key_value_map;
}

typedef enum Node_Type
{
    NODE_TYPE_OBJECT, 
    NODE_TYPE_LIST,
} Node_Type;

typedef struct Ast_Node 
{
    Key_Value_List map_list; // a map from a key to a value.
    Node_Type type;          // can be a list or a regular object. This will determine how to use the key and value list.
} Ast_Node; 

internal Ast_Node *
init_ast_node()
{
    Ast_Node *node = (Ast_Node *)malloc(sizeof(Ast_Node));
    *node = {0};
    return node;
}

typedef struct ParserStringInfo
{
    char *text; 
    b32 handle_key; 
    b32 handle_value;
} StringType;

typedef struct Parser
{
    Lexer lexer;
    ParserStringInfo string_info; 
    Ast_Node *node; // The head node.
} Parser;


internal Ast_Node       *parse_object(Parser *parser);
internal Key_Value_Node *parse_key_value_pair(Parser *parser);

internal Key_Value_Node *
parse_key_value_pair(Parser *parser)
{
    Key_Value_Node *key_value_map = init_key_value_node();
    Lexer *lexer = &parser->lexer;
    Token token;
    
    b32 error = get_next_token(lexer, &token);
    if (error)  goto err;
    if (token.tk_type == TOKEN_STRING)
    {
        key_value_map->key = token.text;
    }
    else 
    {
        //error wrong value type, expected a key type string. got '%' something else.
        return NULL;
    }
    
    error = get_next_token(lexer, &token);
    if (error)  goto err; 
    if (token.tk_type == TOKEN_COLON)
    {
        // Good
    }
    else 
    {
        // bad 
        // I should put an error or something.
        return NULL;
    }
    
    error = get_next_token(lexer, &token);
    if (error)  goto err;
    if (token.tk_type == TOKEN_STRING) // try to figure out the type.
    {
        key_value_map->value = (void *)token.text; 
        key_value_map->value_type = VALUE_TYPE_STRING;
    }
    else if (token.tk_type == TOKEN_INTEGER)
    {
        key_value_map->value = (void *)token.text; 
        key_value_map->value_type = VALUE_TYPE_INTEGER;
    }
    else if (token.tk_type == TOKEN_FLOAT)
    {
        key_value_map->value = (void *)token.text; 
        key_value_map->value_type = VALUE_TYPE_FLOAT;
    }
    else if (token.tk_type == TOKEN_LEFT_CURLY)
    {
        Ast_Node *node = parse_object(parser);
        if (node)
        {
            key_value_map->value = (void *)node; 
            key_value_map->value_type = VALUE_TYPE_OBJECT;
        }
    } // TODO(ziv): Add support for lists and all of its complications. 
    else 
    {
        //error wrong value expected types are string, int, float, object, and list. 
        return NULL;
    }
    
    return key_value_map;
    
    err:
    fprintf(stderr, "something bad happened inside parse_key_value_pair");
    return NULL;
}

internal Ast_Node *
parse_object(Parser *parser)
{
    
    Lexer *lexer = &parser->lexer; 
    Token token; 
    b32 error = get_next_token(lexer, &token); // I know that it begins with a {
    if (error) printf("error in parse object");
    if (token.tk_type == TOKEN_LEFT_CURLY)
    {
        // confirm that the call for this is correct.
        fprintf(stderr, "{ was correct of parsing object");
    }
    
    Key_Value_List key_value_list;
    key_value_list.head = parse_key_value_pair(parser); 
    key_value_list.tail = key_value_list.head;
    
    Ast_Node *node = NULL;
    if (key_value_list.tail)
    {
        error = get_next_token(lexer, &token); 
        while (!error && token.tk_type == TOKEN_COMMA)
        {
            Key_Value_Node *key_value_node = parse_key_value_pair(parser); 
            if (key_value_node)
            {
                key_value_list.tail->next = key_value_node;
                key_value_list.tail = key_value_list.tail->next;
            }
            else
            {
                // error.
                break;
            }
            error = get_next_token(lexer, &token); 
        }
        if (error)
        {
            // something bad has happened.
        }
        else if (token.tk_type != TOKEN_COMMA)
        {
            // ok then I should do something about this.
        }
        
        node = init_ast_node();
        node->map_list = key_value_list;
        node->type = NODE_TYPE_OBJECT;
    }
    
    error = get_next_token(lexer, &token); 
    if (error)
    {
        // error while tokenizing.
    }
    else if (token.tk_type != TOKEN_RIGHT_CURLY)
    {
        // where is the closeing scope
    }
    
    return node;
}

internal Ast_Node *
parse_json(char *input_buffer)
{
#if DEBUG
    init_token_to_string_map();
#endif
    
    Parser parser = {0};
    parser.lexer.text = input_buffer; 
    parser.lexer.location.line = 1; // Most code editors begin at line 1. 
    
    Ast_Node *node = NULL;
    
    Token token = {0}; 
    b32 error = peek_next_token(&parser.lexer, &token);
    while (!error)
    {
#if DEBUG
        debug_print_token(token);
#endif
        
        
        
        if (token.tk_type == TOKEN_LEFT_CURLY)
        {
            node = parse_object(&parser);
        }
        else if (token.tk_type == TOKEN_LEFT_BRACKET)
        {
            // parse list() ... 
        }
        else 
        {
            // error. 
        }
        
        
        error = get_next_token(&parser.lexer, &token);
        
    }
    
    if (error) 
    {
        // we are on a bad time 
    }
    
    return node;
}

internal void 
debug_print_ast_tree(Ast_Node *ast_tree)
{
    Key_Value_Node *head = ast_tree->map_list.head;
    if (ast_tree->type == NODE_TYPE_OBJECT)
    {
        printf("{\n");
        for (; head->next; head = head->next)
        {
            printf("    Key: %s Value: %s,\n", head->key, (char *)head->value);
        }
        printf("    Key: %s Value: %s\n", head->key, (char *)head->value);
        printf("}\n");
    }
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
                debug_print_ast_tree(ast_tree); 
                ast_tree->map_list;
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

/* -- Reaching the finale design of the parser -- */ 

// --- Parsing ---
// At the beginning the design was trying to get 
// away from the recursive nature of the problem. 
// I was doing that because I though that if there
// are many scopes that I need to parse it will 
// quickly eat up the space in the stack and crash.
// Now, before I looked up at ryenflury's datadesk
// project, I thought that scopes could be 
// represented as the count of beginning and so on. 
// As you can see in the code it still was not 
// the solution that I was after. That is because it 
// still relied on the main loop with really hard 
// to code logic as I did not break apart every 
// logical opration withing its own block. 
// Meaning, if I have a key and value pair, they 
// should get parsed together into a block that 
// represents it and then if there is a comma, 
// all I should do is connect this block to the 
// last one. Doing so would grant me a recursive
// function (because, a object can also be in the 
// value of the key value block) that will be easy 
// to program and use within the parser.

// Now, I have yet to test if it works, but if it 
// does, this would be really cool as it would mean
// that a couple of thigs. The first is that parsing
// is really not that hard (considering that I don't 
// do any multithreading and or optimization), and 
// creating simple parsers can be quite nice for a 
// custom format that I might want to use. Second, 
// I have learned how thinking of how to break the 
// problem at hand is super important handling a 
// even remotely hard problem that does not let you 
// hold all of it inside you brain to come up with 
// a good solution, instead, you should break it up 
// as you would like how you would like so that, the 
// bite sized solutions will help you come up with 
// with the correct solution.

// Also, one of my mistakes is to think that the 
// tokenizer should get called only inside the 
// 'json_parser'. This happned because I used 
// some external refrence from the 'sif' compiler. 
// This intern helped me at the beginning but made
// me waste a LOT of time after that, I instead 
// had some ideas that were wrong and I got them 
// because I looked up at others code, I think 
// that even without this I would have though the 
// same but... still, if I can blame someone for 
// of the time that is lost that can only be myself.

/* ------------------------------------------------ */