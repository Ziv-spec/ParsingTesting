/* date = February 21st 2021 11:16 pm */

#ifndef JSON_HL_H
#define JSON_HL_H

/* TODO(ziv):
  Finish the list type in the parser. 
 Continue to add more error messages.
Create a function that converts from my node type to a general hash table.
Create a custom allocator, for many things in the parser that require malloc as of right now.
*/

//
// Logging
// 

internal b32 error_showed; // a global variable that allows or disables output of errors. 
// currently this is a patch for a internal problem, although, it should provide a good method 
// for reuse of code (that exists) and not needing to see the error messages (if I want to do 
// special checks so I could understand the following better).
#define Log(...) fprintf(stdout, __VA_ARGS__); 
#define LogError(...) if(!error_showed) { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); error_showed = true;}

// 
// Lexing
// 

typedef struct Location
{
    s32 index; // index withing the general buffer.
    s32 line;  // line number withing the file. NOTE: begins at 1.
    s32 character; // character at the line.
} Location; 

typedef struct Parser
{
    char *text; // The buffer.
    Location location; 
} Parser; 

enum Token_Types  
{
    TOKEN_NONE,
    TOKEN_LEFT_CURLY,
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

typedef struct Token
{
    Token_Types token_type;
    Location location; 
    char *text; // text for error information.
} Token;

/** Prints out a message that contains the token, where it was found, and the character/text that it contains. This is for debug purposes only, as it is kind of crappy and should most definetly change. 
*/ 
internal void
debug_print_token(Token token)
{
    
    char *token_name = token_name_map[token.token_type];
    
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

internal b32 peek_next_token(Parser *parser, Token *token); 
internal b32 get_next_token(Parser *parser, Token *token_out);

internal b32
get_next_token(Parser *parser, Token *token_out)
{
    
    b32 success = false;
    if (!parser->text[parser->location.index]) { 
        return success; 
    }
    
#define trash_value(value) (value  == ' '|| value == '\n'   || value == '\t')
    char cursor = *(parser->text + parser->location.index); 
    while (trash_value(cursor))
    {
        if (cursor == '\n')
        {
            parser->location.line++;
            parser->location.character = 0;
        }
        parser->location.character++;
        parser->location.index++;
        cursor = *(parser->text + parser->location.index);
    }
    
    s32 token_beginning_index = parser->location.index; // For error messages,
    // I make the problem with the string show up as it's first character 
    //and not the last. For that reason I do this. 
    
    // NOTE(ziv): This should ONLY be used with a *string* in the first argument, and a *token type* in the second argument.
#define TOKENIZE(token_string, is_token_type) if (*(parser->text + parser->location.index) == *token_string) { \
parser->location.index++;                   \
token_out->token_type = is_token_type;      \
token_out->text = token_string;             \
success = true;                             \
}                                          \
    
    TOKENIZE("{",TOKEN_LEFT_CURLY)
        else TOKENIZE("}", TOKEN_RIGHT_CURLY)
        else TOKENIZE(":", TOKEN_COLON)
        else TOKENIZE(",", TOKEN_COMMA) 
        else TOKENIZE("[", TOKEN_LEFT_BRACKET)
        else TOKENIZE("]", TOKEN_RIGHT_BRACKET)
        else if (cursor == '\0')
    {
        success = false;
        return success;
    }
    else 
    {
        string slice_buffer = {0}; 
        
        if (cursor == '"') // tokenize string
        {
            parser->location.index++; // skip the " 
            slice_buffer.data = parser->text + parser->location.index; // beginning of slice.
            cursor = *(parser->text + parser->location.index);
            for (;cursor && cursor != '"'; cursor = *(parser->text + parser->location.index))
            {
                parser->location.index++;
                slice_buffer.size++;
            }
            parser->location.index++; // skip the "
            if (cursor)
            {
                char *token_text = slice_to_string(slice_buffer);
                token_out->text = token_text;
                token_out->token_type = TOKEN_STRING;
                success = true;
            }
            else
            {
                LogError("error tokenizing(%d:%d): no semi colon found to end string.", 
                         parser->location.line, parser->location.character);
                success = false;
            }
            
        }
        else // tokenize integer/float
        {
            slice_buffer.data = parser->text + parser->location.index; // beginning of slice.
            b32 is_floating_point = false;
            
            // Loops until it finds something that is not a number.
            cursor = *(parser->text + parser->location.index);
            for (; is_number(cursor); cursor = *(parser->text + parser->location.index))
            {
                parser->location.index++;
                slice_buffer.size++;
            }
            
            // Is the one before I exited the loop still not a number (did not enter the loop)
            if (is_number(*(parser->text + parser->location.index - 1)))
            {
                if (cursor == '.') // it can be a floating point.
                {
                    is_floating_point = true;
                    
                    parser->location.index++; // skip the '.'
                    slice_buffer.size++;
                    // continues until it finds some bad character
                    cursor = *(parser->text + parser->location.index);
                    for (; is_number(cursor); cursor = *(parser->text + parser->location.index))
                    {
                        parser->location.index++;
                        slice_buffer.size++;
                    }
                    
                    // TODO(ziv): Fix this !!! it picks up on wrong things.
                    // the real solution should be a peek_next_token call 
                    // that for the time begin does not work as it thinks 
                    // that random garbege is a number. The solution
                    // should be a more robust numbers identifire that is 
                    // able to better understand numbers and so on so bugs
                    // like these will not happen an as such the log report
                    // here was patched in a bad manner instead of fixing 
                    // the problem.
                    if (!is_number(*(parser->text + parser->location.index - 1)))
                    {
                        LogError("error while tokenizing (%d:%d): unknown value.", 
                                 parser->location.line, parser->location.character);
                        return success;
                    }
                    
                }
                else // it can also be some garbage.
                {
                    if (!is_number(cursor))
                    {
                        LogError("error tokenizing(%d:%d): expected number got unknown value.", 
                                 parser->location.line, parser->location.character);
                        success = false;
                        return success;
                    }
                }
            }
            token_out->text = slice_to_string(slice_buffer);
            success = true;
            if (is_floating_point)
            {
                token_out->token_type = TOKEN_FLOAT;
            }
            else
            {
                token_out->token_type = TOKEN_INTEGER;
            }
        }
        
    }
    
    // synces the out token location.  
    token_out->location = parser->location; 
    parser->location.character += (parser->location.index - token_beginning_index);
    
#if DEBUG
    debug_print_token(*token_out);
#endif
    
    return success;
}

internal b32
peek_next_token(Parser *parser, Token *token)
{
    Location old_location = parser->location;
    b32 success = get_next_token(parser, token); 
    parser->location = old_location;
    return success;
}

//
// Parsing
//

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

internal Ast_Node       *parse_object(Parser *parser);
internal Key_Value_Node *parse_key_value_pair(Parser *parser);

internal Key_Value_Node *
parse_key_value_pair(Parser *parser)
{
    Key_Value_Node *key_value_map = init_key_value_node();
    
    Token token;
    b32 success = get_next_token(parser, &token);
    if (!success)  return NULL;
    char *key_text = NULL;
    if (token.token_type == TOKEN_STRING)
    {
        key_text = token.text;
        key_value_map->key = token.text;
    }
    else 
    {
        //error wrong value type, expected a key type string. got '%' something else.
        return NULL;
    }
    
    success = get_next_token(parser, &token);
    if (!success) return NULL;
    if (token.token_type != TOKEN_COLON) 
    {
        LogError("error parsing(%d:%d): after key '%s' expected to see COLON, got '%s'.", 
                 parser->location.line, parser->location.character, key_text, token.text);
        return NULL;
    }
    
    // figure out the value type and put it 
    success = get_next_token(parser, &token);
    if (!success)  return NULL;
    
    if (token.token_type == TOKEN_STRING) // try to figure out the type.
    {
        key_value_map->value = (void *)token.text; 
        key_value_map->value_type = VALUE_TYPE_STRING;
    }
    else if (token.token_type == TOKEN_INTEGER)
    {
        key_value_map->value = (void *)token.text; 
        key_value_map->value_type = VALUE_TYPE_INTEGER;
    }
    else if (token.token_type == TOKEN_FLOAT)
    {
        key_value_map->value = (void *)token.text; 
        key_value_map->value_type = VALUE_TYPE_FLOAT;
    }
    else if (token.token_type == TOKEN_LEFT_CURLY)
    {
        Ast_Node *node = parse_object(parser);
        if (node)
        {
            key_value_map->value = (void *)node; 
            key_value_map->value_type = VALUE_TYPE_OBJECT;
        }
    }   // TODO(ziv): Add support for lists and all of its complications. 
    else 
    {
        LogError("error parsing(%d:%d): expected a value of types string/integer/float/object/list, got unknown value '%s'.", 
                 parser->location.line, parser->location.character, token.text);
        return NULL;
    }
    
    return key_value_map;
}

internal Ast_Node *
parse_object(Parser *parser)
{
    // TODO(ziv): Make this more robust, handle the errors that might come (although there shouldn't be many).
    Token token; 
    
    Key_Value_List key_value_list;
    key_value_list.head = parse_key_value_pair(parser); 
    key_value_list.tail = key_value_list.head;
    b32 success = false;
    Ast_Node *node = NULL;
    if (key_value_list.tail)
    {
        success = get_next_token(parser, &token); 
        while (success && token.token_type == TOKEN_COMMA)
        {
            Key_Value_Node *key_value_node = parse_key_value_pair(parser); 
            if (key_value_node)
            {
                key_value_list.tail->next = key_value_node;
                key_value_list.tail = key_value_list.tail->next;
            }
            else
            {
                // success.
                break;
            }
            success = get_next_token(parser, &token); 
        }
        if (!success)
        {
            // something bad has happened.
        }
        else if (token.token_type != TOKEN_COMMA)
        {
            // ok then I should do something about this.
        }
        
        node = init_ast_node();
        node->map_list = key_value_list;
        node->type = NODE_TYPE_OBJECT;
    }
    
    success = get_next_token(parser, &token); 
    if (!success)
    {
        // error while tokenizing.
    }
    else if (token.token_type != TOKEN_RIGHT_CURLY)
    {
        // where is the closeing scope gone to? 
        // error, no closing scope
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
    parser.text = input_buffer; 
    parser.location.line = 1; // Most code editors begin at line 1. 
    
    Ast_Node *node = NULL;
    
    Token token = {0}; 
    b32 success = get_next_token(&parser, &token);
    while (success)
    {
        
        if (token.token_type == TOKEN_LEFT_CURLY)
        {
            node = parse_object(&parser);
            break;
        }
        else if (token.token_type == TOKEN_LEFT_BRACKET)
        {
            // parse list() ... 
        }
        else 
        {
            // error. 
        }
        
        success = get_next_token(&parser, &token);
        
    }
    
    
    return node;
}

// TODO(ziv): Make this Robust and make it support whatever.
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

#endif //JSON_HL_H