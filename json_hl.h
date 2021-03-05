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
#define LogError(...) if(!error_showed) { fprintf(stderr, __VA_ARGS__); fprintf(stderr, " %d", __LINE__); fprintf(stderr, "\n"); error_showed = true;}

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
    TOKEN_UNKNOWN,
    TOKEN_COUNT
};

static char *token_name_map[TOKEN_COUNT]; 

void init_token_to_string_map()
{
    token_name_map[TOKEN_LEFT_CURLY]    = "LEFT_CURLY"; 
    token_name_map[TOKEN_RIGHT_CURLY]   = "RIGHT_CURLY"; 
    token_name_map[TOKEN_RIGHT_BRACKET] = "RIGHT_BRACKET"; 
    token_name_map[TOKEN_LEFT_BRACKET]  = "LEFT_BRACKET"; 
    token_name_map[TOKEN_SEMI_COLEN]    = "SEMI_COLEN";
    token_name_map[TOKEN_COLON]         = "COLON";
    token_name_map[TOKEN_COMMA]         = "COMMA"; 
    
    token_name_map[TOKEN_INTEGER]       = "INTEGER"; 
    token_name_map[TOKEN_STRING]        = "STRING"; 
    token_name_map[TOKEN_FLOAT]         = "FLOAT"; 
}

typedef struct Token
{
    Token_Types token_type;
    Location location; 
    char *text;   // text for error information.
	s32 text_size;// the size of the text.
} Token;

/** Prints out a message that contains the token, where it was found, and the character/text 
that it contains. This is for debug purposes only, as it is kind of crappy and should most
 definetly change.
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
    
    // 'cursor' is my current location (at all times) in the text buffer. 
    // as such it is a pointer to that text buffer, not a character.
#define cursor (parser->text + parser->location.index)
    
    if (!(*cursor))  return success; // make sure that I am not at the end
    
    // skip all values that have not meaning (in any context).
#define trash_value(value) (value  == ' ' || value == '\n' || value == '\t')
    while (trash_value(*cursor))
    {
        if (*cursor == '\n')
        {
            parser->location.line++;
            parser->location.character = 0;
        }
        parser->location.character++;
        parser->location.index++;
    }
    
    // NOTE(ziv): This should ONLY be used with a *string* in the first argument, and a *token type* in the second argument.
#define TOKENIZE(token_string, is_token_type) if (*cursor == *token_string) { \
parser->location.index++;                   \
parser->location.character++;               \
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
        else if (*cursor == '"') // TOKEN_STRING
    {
        string slice_buffer = {0}; 
        parser->location.index++; // skip the beginning " 
		slice_buffer.data = cursor;
		
        //parser->location.character++; 
        
		while (*cursor && *cursor != '"' && *cursor != '\n')
        {
            slice_buffer.size++;
            parser->location.index++;
            parser->location.character++;
        }
		if (*cursor == '\n')
		{
			LogError("error tokenizing(%d:%d): expected closing semi colon for a STRING type.", 
					 parser->location.line, parser->location.character);
		}
		else if (*cursor)
        {
            parser->location.index++; // skip the ending "
            
            char *token_text = slice_to_string(slice_buffer);
            token_out->text = token_text;
            token_out->token_type = TOKEN_STRING;
            success = true;
        }
        else // the cursor is a nullterminator aka end of file.  
        {
            LogError("error tokenizing(%d:%d): end of file.\nThis is probably due to a missing semi colon.", 
                     parser->location.line, parser->location.character);
            return false;
        }
        
    }
    else if (is_number(*cursor)) // TOKEN_INTEGER or TOKEN_FLOAT
    {
        string slice_buffer = {0};
        slice_buffer.data = cursor; // beginning of slice.
        
        // Loops until it finds something that is not a number.
        while (is_number(*cursor))
        {
            slice_buffer.size++;
            parser->location.index++;
        }
        
        if (*cursor == '.') 
        {
            parser->location.index++; // skip the '.'
            slice_buffer.size++;
            
            while (is_number(*cursor))
            {
                slice_buffer.size++;
                parser->location.index++;
            }
            token_out->text = slice_to_string(slice_buffer);
            token_out->token_type = TOKEN_FLOAT;
            success = true;
        }
        else
        {
            
            Token next_token;
            if (peek_next_token(parser, &next_token))
            {
                if (next_token.token_type == TOKEN_UNKNOWN)
                {
                    get_next_token(parser, &next_token); // eat the next token.
					
					Assert(slice_buffer.data != NULL || next_token.text != NULL);
                    
                    s32 src1_size = slice_buffer.size;
                    s32 src2_size = string_size(next_token.text);
                    s32 dest_size = src1_size + src2_size + 1; 
                    char *dest_buffer = (char *)malloc(dest_size);
                    
                    if (strmerge(dest_buffer, dest_size, 
                                 slice_buffer.data, src1_size, 
                                 next_token.text, src2_size))
                    {
                        LogError("error tokenizing(%d:%d): unknown value '%s'.", 
                                 parser->location.line, parser->location.character, dest_buffer);
					}
                }
                else
                {
					token_out->text = slice_to_string(slice_buffer); 
					token_out->token_type = TOKEN_INTEGER;
					success = true;
					//LogError("error tokenizing(%d:%d): unknown type to handle. parser is unable to understand what is going on, aka this is a bug. ",
					//parser->location.line, parser->location.character);
				}
            }
            
        }
    }
    else if (*cursor == '\0')
    {
        token_out->token_type = TOKEN_NONE;
        token_out->text = '\0';
        return false;
    }
    else // TOKEN_UNKNOWN
    {
#define MAX_BUFFER_SIZE 255
        Token trash_token = {0}; // TOKEN_UNKNOWN 
		char dest_text[MAX_BUFFER_SIZE];
		s32 character_index = 0;
		
		while (peek_next_token(parser, &trash_token) && 
			   trash_token.token_type == TOKEN_UNKNOWN)
        {
            get_next_token(parser, &trash_token); // eat the token.
			dest_text[character_index++] = *trash_token.text; 
			if (character_index > MAX_BUFFER_SIZE)
			{
				Log("255 character is not enough for some reason, if you can explain to me why, then go ahead.");
			}
		}
        
    }
    
    // synces the out token location.  
    token_out->location = parser->location; 
    
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
    Token token;
    if (!get_next_token(parser, &token))  return NULL;
    
    Key_Value_Node *key_value_map = init_key_value_node();
    char *key_text = NULL;  // for error prints. 
    if (token.token_type == TOKEN_STRING)
    {
        key_text = token.text;
        key_value_map->key = token.text;
    }
    else 
    {
        LogError("error parsing(%d:%d): expected a key of type 'STRING', got type '%s'.", 
                 parser->location.line, parser->location.character, token_name_map[token.token_type]); 
        return NULL;
    }
    
    if (!get_next_token(parser, &token))  return NULL;
    if (token.token_type != TOKEN_COLON) 
    {
        LogError("error parsing(%d:%d): after key '%s' expected COLON, got '%s'.", 
                 parser->location.line, parser->location.character, key_text, token.text);
        return NULL;
    }
    
    // figure out the value type and put it 
    if (!get_next_token(parser, &token))  return NULL;
    
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
		key_value_map->value = node; 
		key_value_map->value_type = VALUE_TYPE_OBJECT;
    }   // TODO(ziv): Add support for lists and all of its complications. 
    else 
    {
        LogError("error parsing(%d:%d): expected VALUE of type OBJECT/LIST/STRING/INTEGER/FLOAT got unknown value '%s'.", 
                 parser->location.line, parser->location.character, token.text);
        return NULL;
    }
    
    return key_value_map;
}

internal Ast_Node *
parse_object(Parser *parser)
{
    Token token; 
    b32 success = false;
    Ast_Node *node = NULL;
    
	// empty object.
	if (peek_next_token(parser, &token) && token.token_type == TOKEN_RIGHT_CURLY)
	{
		get_next_token(parser, &token);
		return NULL;
	}
	
    Key_Value_List key_value_list;
    Key_Value_Node *key_value_node = parse_key_value_pair(parser); // if null the object is empty
    key_value_list.head = key_value_node;
    key_value_list.tail = key_value_list.head;
	
    success = get_next_token(parser, &token);
	while (key_value_node && success && 
		   token.token_type == TOKEN_COMMA)
    {
        key_value_node = parse_key_value_pair(parser);
        key_value_list.tail->next = key_value_node;
        key_value_list.tail = key_value_list.tail->next;
		
        success = get_next_token(parser, &token);
    }
    
	if (success) 
	{
		if (token.token_type == TOKEN_RIGHT_CURLY)
		{
			node = init_ast_node();
			node->map_list = key_value_list;
			node->type = NODE_TYPE_OBJECT;
		}
		else
		{
			LogError("error parsing(%d:%d): expected right curly bracked, got '%s'.", 
					 parser->location.line, parser->location.character, token.text)
		}
	}
	else
	{
		// a problem while tokenizing happened.
		return NULL;
	}
	
	return node;
}

internal Ast_Node *
parse_json(char *input_buffer)
{
	init_token_to_string_map();
	
	Parser parser = {0};
	parser.text = input_buffer; 
	parser.location.line = 1; // Most code editors begin at line 1. 
	
	Ast_Node *node = NULL;
	
	Token token = {0}; 
	b32 success = get_next_token(&parser, &token);
	
	if (token.token_type == TOKEN_LEFT_CURLY)
	{
		node = parse_object(&parser);
	}
	else if (token.token_type == TOKEN_LEFT_BRACKET)
	{
		// node = parse_list(&parser); ... 
	}
	else 
	{
		LogError("error parsing(%d:%d): did not find object/list type at the beginning", parser.location.line, parser.location.character);
	}
	
	return node;
}


// some really quick witted degub printing for the Ast_Node. 

internal void 
debug_print_padding(s32 padding_level)
{
	for (int padding_counter = 0; padding_counter < padding_level; padding_counter++)
	{
		Log("\t");
	}
}

internal void debug_print_object(Key_Value_Node *key_value_map, s32 padding_level);

internal void
debug_print_value(void *value, Value_Type value_type, s32 padding_level)
{
	
	if (value_type == VALUE_TYPE_OBJECT)
	{
		Ast_Node *node = (Ast_Node *)value;
		if (node)  debug_print_object(node->map_list.head, padding_level);
		else Log("{}");
	}
	else 
	{
		//debug_print_padding(padding_level);
		if (value_type == VALUE_TYPE_STRING)
		{
			Log("%c%s%c", '"',(char *)value, '"'); 
		}
		else if (value_type == VALUE_TYPE_INTEGER)
		{
#define integer_to_string(a) a 
			Log(integer_to_string((char *)value));
		}
		else if (value_type == VALUE_TYPE_FLOAT)
		{
#define float_to_string(a) a
			Log(float_to_string((char *)value));
		}
		else 
		{
			// I don't handle this. 
		}
	}
}

internal void
debug_print_key_value_pair(Key_Value_Node *kv_node, s32 padding_level)
{
	debug_print_padding(padding_level); Log("%c%s%c",'"', kv_node->key, '"'); 
	Log(": "); debug_print_value(kv_node->value, 
								 kv_node->value_type, 
								 padding_level); 
}


internal void
debug_print_object(Key_Value_Node *key_value_map, s32 padding_level)
{
	Log("{\n"); 
	if (key_value_map)
	{
		for (; key_value_map->next; key_value_map = key_value_map->next)
		{
			debug_print_key_value_pair(key_value_map, padding_level + 1);
			Log(", \n");
		}
		debug_print_key_value_pair(key_value_map, padding_level + 1); Log("\n");
		debug_print_padding(padding_level); 
	}
	Log("}");
}

internal void 
debug_print_json_from_ast_node(Ast_Node *node)
{
	if (node)
	{
		Key_Value_List kv_map = node->map_list; 
		if (node->type == NODE_TYPE_OBJECT)
		{
			debug_print_object(kv_map.head, 0);
		}
		else
		{
			// I cannot handle this for the moment.
		}
	}
}

#endif //JSON_HL_H