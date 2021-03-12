/* date = February 21st 2021 11:16 pm */

#ifndef JSON_HL_H
#define JSON_HL_H

/* TODO(ziv):
  
  - @working Finish the list type in the parser. 
  Continue to add more error messages.
 Create a function that converts from my node type to a general hash table.
s@Cleanup for the enums as I have toom nay of them nad I don't really need it. 
*/

//
// Types for convenience.
// 

// NOTE(ziv): This should get changed. This is because, 
// I no like to override types for others. Instead, 
// the types that the project will have different name 
// so I there will be no naming colissions.

#include <stdint.h>
typedef int8_t  s8; 
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

#if DEBUG
#define Assert(expression) if(!(expression)) { *(int *)0 = 0; } 
#else
#define Assert(expression)
#endif

// returns true or false if a character is a number.
#define is_number(value) ('0' <= value && value <= '9')

// This assumes that if you use the unity compilation model
// you use the internal keyword. If not, you can thank me later :)
// This is done this way so if you don't use the internal 
// keyword it won't affect you really, but it you do, it will 
// bring you benefits.
#ifndef internal
#define internal
#else
#undef internal 
#define internal static
#endif

// for global variables. 
#ifndef global
#define global
#else 
#undef  global 
#define global static
#endif 

#define size_t s64


////////////////////////////////
// 
// Strings
// 

typedef union 
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

internal b32   string_compare(char *value1, char *value2); /* compares between two null terminated strings */ 
internal s32   string_size(char *string);          /* returns the nullterminated strings size */
internal char *slice_to_string(string slice);      /* converts a slice type to a null terminated character array */
internal b32   strmerge(char *dest, s32 dest_size, /* merges between two strings into a dest string */
						const char *src1, s32 size1, 
						const char *src2, s32 size2);


////////////////////////////////
//
// Logging
// 

internal b32 error_showed; // a global variable that allows or disables output of errors. 
// currently this is a patch for a internal problem, although, it should provide a good method 
// for reuse of code (that exists) and not needing to see the error messages (if I want to do 
// special checks so I could understand the following better).
#define Log(...) fprintf(stdout, __VA_ARGS__); 
#define LogError(...) if(!error_showed) { fprintf(stderr, __VA_ARGS__); fprintf(stderr, " %d", __LINE__); fprintf(stderr, "\n"); error_showed = true;}


////////////////////////////////
// 
// Memory Management
// 

// The memory is handled in a big pool. 
// A pool of 4MB of memory is created, 
// and then all memory is accessed and 
// allocated there. If there is not 
// enough memory it will allocate one 
// more pool. This pool allocation allows
// for fast allocations and for easy 
// freeing of memory as a pool instead 
// of individualy.

typedef struct Memory_Block
{
	struct Memory_Block *next; // next memory pool.
	size_t  buffer_capacity;   // size of the memory buffer.
	size_t  offset;            // where the last allocation happened.
	void   *buffer;            // memory buffer. 
} Memory_Block;

typedef struct Memory_Pool
{
	Memory_Block *head; 
	Memory_Block *tail; 
} Memory_Pool;

typedef struct Context
{
	void *(*alloc)(s32);   // function for allocating memory. 
	void  (*free)(void);   // function for freeing the allocated memory.
	Memory_Pool memory;    // the default memory pool
	s32   allocation_size; // the default size for each block of memory.
} Context; 

global Context context;

//internal void init_global_context(s32 pool_size); 

internal void *
json_memory_alloc(s32 requested_size)
{
	Memory_Block *last_block = context.memory.tail;
	
	if ((last_block->offset + requested_size) < last_block->buffer_capacity)
	{
		void *requested_memory = (u8 *)last_block->buffer + last_block->offset;
		last_block->offset += requested_size; 
		return requested_memory;
	}
	else // not enough space in the buffer, there is a need for a new allocation. 
	{
		Memory_Block *memory_block = (Memory_Block *)malloc(sizeof(Memory_Block));
		if (memory_block)
		{
			context.memory.tail->next   = memory_block;  
			context.memory.tail->buffer = malloc(context.allocation_size);
			return memory_block;
		}
		else 
		{
			// cannot allocate memory.
			return NULL;
		}
	}
}

internal void
json_memory_free()
{
	Memory_Block *block = context.memory.head;
	Memory_Block *next_block = block->next;
	
	while (block->next)
	{
		next_block = block->next;
		free(block); 
		block = next_block;
	}
	if (block)
	{
		free(block);
	}
	
}

internal void 
init_global_context(s32 block_size)
{
	context.alloc = &json_memory_alloc;
	context.free =  &json_memory_free;
	
	context.allocation_size = block_size;
	
	s32 total_allocation_size = sizeof(Memory_Block) + context.allocation_size;
	context.memory.head = (Memory_Block *)malloc(total_allocation_size);
	*context.memory.head = {0};
	context.memory.head->buffer_capacity = context.allocation_size;
	context.memory.head->buffer = (u8 *)context.memory.head + sizeof(Memory_Block); 
	context.memory.tail = context.memory.head;
}





////////////////////////////////
// 
// Lexing
// 

typedef struct Location
{
    s32 index;     // index withing the general buffer.
    s32 line;      // line number withing the file.
    s32 character; // character at the line.
} Location; 

typedef struct Parser
{
    char *text; // The buffer.
    Location location; // parsers location in the text buffer.
} Parser; 

enum Token_Types // all characters known to the parser.
{
    TOKEN_NONE          = 0,   // 0x0
    TOKEN_LEFT_CURLY    = 1,   // 0x1
    TOKEN_RIGHT_CURLY   = 2,   // 0x2
    TOKEN_RIGHT_BRACKET = 4,   // 0x4 
    TOKEN_LEFT_BRACKET  = 8,   // 0x8
    TOKEN_SEMI_COLEN    = 16,  // 1x0 
    TOKEN_COLON         = 32,  // 2x0
    TOKEN_COMMA         = 64,  // 4x0
    TOKEN_INTEGER       = 128, // 8x0
    TOKEN_STRING        = 256, // 10x0 
    TOKEN_FLOAT         = 512, // 20x0
    TOKEN_UNKNOWN       = 1024 // 40x0
};

typedef struct Token
{
    Token_Types token_type; // Type of the token.
    Location location;      // location of the token in the text buffer (inside the parser).
    char *text;   // text for the token.
	s32 text_size;// the size of the text.
} Token;

#define TOKEN_COUNT 13 // Only for the map down below.
static char *token_name_map[TOKEN_COUNT]; 

internal void
init_token_to_string_map()
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


internal b32 get_next_token(Parser *parser,    // advances the parsers location.
							Token *token_out); // the token will get put here.
// If there is no token (or it is unknown), it returns 0.

internal b32 peek_next_token(Parser *parser,    // does not advance the parsers location. 
							 Token *token_out); // the token will get put here.
// If there is no token (or it is unknown), it returns 0.

internal void debug_print_token(Token token); // prints the token given.


////////////////////////////////
//
// Parsing
//

typedef enum Value_Type
{
    VALUE_TYPE_NONE    = 1,
    VALUE_TYPE_OBJECT  = 2,
    VALUE_TYPE_LIST    = 4,
    VALUE_TYPE_STRING  = 8,
    VALUE_TYPE_INTEGER = 16, 
    VALUE_TYPE_FLOAT   = 32
} Value_Type;

typedef struct Key_Value_Node
{
    char *key;            // key 
    void *value;          // value of any type that the enum Value_Type has. 
	s32 value_type_flag;  // flags are of type Value_Type that tell what type is the value.
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
    Key_Value_Node *key_value_map = (Key_Value_Node *)context.alloc(sizeof(Key_Value_Node)); 
    key_value_map->key = NULL; 
    key_value_map->value = NULL; 
    key_value_map->value_type_flag = VALUE_TYPE_NONE; 
    key_value_map->next = NULL;
    return key_value_map;
}


////////////////////////////////


typedef enum Node_Type
{
    NODE_TYPE_OBJECT, 
    NODE_TYPE_LIST
} Node_Type;

typedef struct Ast_Node 
{
    Key_Value_List map_list; // a map from a key to a value.
    Node_Type type;          // can be a list or a regular object. This will determine how to use the key and value list.
} Ast_Node; 

internal Ast_Node *
init_ast_node()
{
    Ast_Node *node = (Ast_Node *)context.alloc(sizeof(Ast_Node));
    *node = {0};
    return node;
}

// The following functions parse known types. 
// They handle their own errors, and return NULL 
// when an error has occored. 
internal Key_Value_Node *parse_key_value_pair(Parser *parser);
internal Ast_Node       *parse_object(Parser *parser);
internal Ast_Node       *parse_list(Parser *parser);

internal Ast_Node *parse_json(char *input_buffer);


////////////////////////////////


enum Simple_Types
{
	String  = 1, 
	Float   = 2,  
	Object  = 4, 
	Array   = 8, 
	Integer = 16
};

typedef struct Complex_Type_Node
{
	s32 structure_flags;
	struct Complex_Type_Node *next; 
} Complex_Type_Node;

typedef struct Complex_Type
{
	Complex_Type_Node *head; 
	Complex_Type_Node *tail; 
} Complex_Type; 

internal Complex_Type 
create_complex_type_from_object(Ast_Node *obj)
{
	// TODO(ziv): go through and check this again.
	Complex_Type complex_type_head; 
	complex_type_head.head = (Complex_Type_Node *)context.alloc(sizeof(Complex_Type_Node)); 
	*complex_type_head.head = {0}; 
	
	complex_type_head.tail = complex_type_head.head;
	
	Key_Value_Node *pairs_head = obj->map_list.head; 
	Complex_Type_Node *type_head = complex_type_head.head;
	
	// TODO(ziv): Fix the bug in here. If 'pairs_head' is NULL, there is going to be an error here.
	
	// Go through each pairs_head and add that type to the complex_type_list. 
	while (pairs_head->next)
	{
		type_head->structure_flags |= pairs_head->value_type_flag;
		type_head->next = (Complex_Type_Node *)context.alloc(sizeof(Complex_Type_Node)); 
		type_head->next->structure_flags = 0;
		
		type_head  = type_head->next;
		pairs_head = pairs_head->next;
	}
	
	if (pairs_head)
	{
		type_head->structure_flags |= pairs_head->value_type_flag;
		type_head->next = NULL;
		
		type_head  = type_head->next;
		pairs_head = pairs_head->next;
	}
	
	return complex_type_head; 
}

internal b32 
compare_object_to_complex_type(Ast_Node *obj, Complex_Type type)
{
	Key_Value_Node *obj_head = obj->map_list.head; 
	Complex_Type_Node *type_head = type.head; 
	
	while (type_head && obj_head)
	{
		if ((obj_head->value_type_flag) != type_head->structure_flags)
		{
			// will need to throw an error.
			return false;
		}
		type_head = type_head->next; 
		obj_head = obj_head->next; 
	}
	return true;
}


////////////////////////////////

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
    
    // This should ONLY be used with a *string* in the first argument, and a *token type* in the second argument.
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
                    char *dest_buffer = (char *)context.alloc(dest_size);
                    
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
peek_next_token(Parser *parser, Token *token_out)
{
    Location old_location = parser->location;
    b32 success = get_next_token(parser, token_out); 
    parser->location = old_location;
    return success;
}

internal Key_Value_Node *
parse_key_value_pair(Parser *parser)
{
    Token token;
    if (!get_next_token(parser, &token))  return NULL;
    
	//
	// checks for a key
	//
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
    
	//
	// check if there is a colon 
	//
    if (!get_next_token(parser, &token))  return NULL;
    if (token.token_type != TOKEN_COLON) 
    {
        LogError("error parsing(%d:%d): after key '%s' expected COLON, got '%s'.", 
                 parser->location.line, parser->location.character, key_text, token.text);
        return NULL;
    }
	
    //
    // checks for a value
	//
    if (!get_next_token(parser, &token))  return NULL;
    
    if (token.token_type == TOKEN_STRING) // try to figure out the type.
    {
        key_value_map->value = (void *)token.text; 
        key_value_map->value_type_flag = VALUE_TYPE_STRING;
    }
    else if (token.token_type == TOKEN_INTEGER)
    {
        key_value_map->value = (void *)token.text; 
        key_value_map->value_type_flag = VALUE_TYPE_INTEGER;
    }
    else if (token.token_type == TOKEN_FLOAT)
    {
        key_value_map->value = (void *)token.text; 
        key_value_map->value_type_flag = VALUE_TYPE_FLOAT;
    }
    else if (token.token_type == TOKEN_LEFT_CURLY)
    {
        Ast_Node *node = parse_object(parser);
		key_value_map->value = node; 
		key_value_map->value_type_flag = VALUE_TYPE_OBJECT;
    }
	else if (token.token_type == TOKEN_LEFT_BRACKET)
	{
		Ast_Node *node =  parse_list(parser); 
		key_value_map->value = node; 
		key_value_map->value_type_flag = VALUE_TYPE_LIST;
	}
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
parse_list(Parser *parser)
{
	Token token; 
    b32 success = false;
	
	// empty list.
	if (peek_next_token(parser, &token) && token.token_type == TOKEN_RIGHT_BRACKET)
	{
		get_next_token(parser, &token); // eat that token.
		return NULL;
	}
	
	Ast_Node *node = init_ast_node();
	Complex_Type complex_type = {0};
	
	success = get_next_token(parser, &token);
	if (token.token_type == TOKEN_LEFT_CURLY) // OBJECT
	{
		
		Ast_Node *obj = parse_object(parser); 
		if (!obj) return NULL;
		
		complex_type = create_complex_type_from_object(obj); 
		
		success =  get_next_token(parser, &token);
		while (obj && success && token.token_type == TOKEN_COMMA)
		{
			obj = parse_object(parser);
			if (!compare_object_to_complex_type(obj, complex_type))
			{
				// TODO(ziv): compelete this once you actually do this.
				LogError("error parsing(%d:%d): missmatch of types inside an ARRAY ('' != '')", 
						 parser->location.line, parser->location.character);
				return NULL; 
			}
			
			success = get_next_token(parser, &token);
		}
		
	}
	else if (token.token_type == TOKEN_STRING) // STRING
	{
		char *text = token.text; 
		
	}
	else if (token.token_type == TOKEN_INTEGER) // INTEGER
	{
		
	}
	else if (token.token_type == TOKEN_FLOAT) // FLOAT
	{
		
	}
	else if (token.token_type == TOKEN_LEFT_BRACKET) // ARRAY
	{
		
	}
	else // Some unknown type.
	{
		return NULL; // some problem has occured.
	}
	
	
	
	/* 	
		do 
		{
			
		} while (success && (token.token_type & parser->list_type_flags));
		 */
	
	return node;
}

internal Ast_Node *
parse_json(char *input_buffer)
{
	init_token_to_string_map();
	init_global_context(8 * 1024);
	
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
		node = parse_list(&parser);
	}
	else 
	{
		LogError("error parsing(%d:%d): did not find object/list type at the beginning", parser.location.line, parser.location.character);
	}
	
	context.free();
	
	return node;
}

//
// String Functions Implementation
// 

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
    
    char *result = (char *)context.alloc(slice.size+1); 
    char *temp_char = result; 
    for (s32 counter = 0; counter < slice.size; counter++)
    {
        *temp_char++ = *slice.data++;
    }
    *temp_char = '\0';
    return result;
}


internal b32
strmerge(char *dest, s32 dest_size, 
         const char *src1, s32 size1, 
         const char *src2, s32 size2)
{
    if (!dest || !src1 || !src2 || 
        dest_size > (size1 + size2))  return false;
    
    for (; *src1; src1++)
    {
        *dest++ = *src1;
    }
    
    for (; *src2; src2++)
    {
        *dest++ = *src2;
    }
    *dest++ = '\0';
    
    return true;
}







// some really quick witted debug printing for the Ast_Node. 
internal void debug_print_object(Key_Value_Node *key_value_map, s32 padding_level);

internal void 
debug_print_padding(s32 padding_level)
{
	for (int padding_counter = 0; padding_counter < padding_level; padding_counter++)
	{
		Log("\t");
	}
}


internal void
debug_print_value(void *value, s32 value_type, s32 padding_level)
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
								 kv_node->value_type_flag, 
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

#endif //JSON_HL_H