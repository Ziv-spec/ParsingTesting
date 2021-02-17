/* date = February 12th 2021 7:07 pm */

#ifndef LEXER_H
#define LEXER_H

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
    s32 longest_size = string_size("TOKEN_RIGHT_BRACKET");
    s32 token_string_size = string_size(token_name);
    int counter = 0;
    for (; counter < longest_size - token_string_size; counter++)
    {
        padding[counter] = ' ';
    }
    padding[counter] = '\0';
    
    printf("%s%s%d:%d \n", token_name,padding, token.location.line, token.location.character); 
}

enum Lexer_Error
{
    TOKEN_MESSAGE_SUCCESS = 0, // should be zero so 'if (!error)' would be possible.
    TOKEN_MESSAGE_STRING  = 1,
    TOKEN_MESSAGE_INTEGER = 2, 
    TOKEN_MESSAGE_FLOAT   = 3,
    TOKEN_MESSAGE_END     = 4,  
} Lexer_Error_Code; 

inline b32
is_number(char value)
{
    return '0' <= value && value <= '9';
}

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

#endif //LEXER_H
