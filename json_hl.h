/* date = February 21st 2021 11:16 pm */

#ifndef JSON_HL_H
#define JSON_HL_H

// NOTE(ziv): The grand plan: 
// Fix these functions first

// [x] parse_json
// [x] parse_key_value_pair
// [x] parse_object 
// [ ] parse_array
// [ ] parse_array_basic_type <- what is this??? 


/* TODO(ziv):
  - @working Finish the list type in the parser. 
  Continue to add more error messages.
 Create a function that converts from my node type to a general hash table.

@Cleanup for the enums as I have toom nay of them nad I don't really need it. 
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
// if the value is a number then it can convert it to an integer.
#define to_number(value) (value - '0')

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

//#define size_t s64

#include <string.h> // memcpy
#include <stdio.h>  // printf, fopen..
#include <stdlib.h> // malloc

////////////////////////////////
// 
// Strings
// 

typedef union
{
    struct 
    {
        s32 size;
        char *data;
    };
    
    struct
    {
        s32 index;
        char *data; 
    };
	
	struct 
	{
		s32 capacity; 
		char *data;
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
	size_t buffer_capacity;   // size of the memory buffer.
	size_t offset;            // where the last allocation happened.
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
	void *(*realloc)(void *, size_t); // function for reallcating memory.
	Memory_Pool memory;    // the default memory pool
	s32   allocation_size; // the default size for each block of memory.
} Context; 

global Context context;

//internal void init_global_context(s32 pool_size); 

internal void *
json_memory_alloc(s32 requested_size)
{
	Memory_Block *last_block = context.memory.tail;
	
	// check if there is enough space in the block
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

// TODO(ziv): evaluate whether I truly need this !!! 

/* 
internal void *
json_memory_realloc(const void *old_memory, size_t old_size, size_t new_size)
{
	
	
}
 */

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

typedef enum Token_Types // all characters known to the parser.
{
    TOKEN_UNKNOWN       = 0,   // 0x0
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
    TOKEN_NONE          = 1024 // 40x0
} Token_Types;

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

// General types that the json format specifies.
typedef enum Simple_Types
{
	TYPE_String  = 1, 
	TYPE_Float   = 2,  
	TYPE_Integer = 8, 
	TYPE_Object  = 4, 
	TYPE_Array   = 16, 
	TYPE_Null    = 32
} Simple_Types;

typedef struct Key_Value_Node
{
    char *key;   // key 
    void *value; // value of any type that the enum Value_Type has. 
	Simple_Types value_type_flag;  // flags are of type Value_Type that tell what type is the value.
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
    key_value_map->value_type_flag = (Simple_Types)0; 
    key_value_map->next = NULL;
    return key_value_map;
}


typedef struct Json_Type
{
	Simple_Types type;
	
	union 
	{
		struct {
			size_t capacity;
			size_t index;
			Json_Type *data;
		} array; 
		
		struct {
			char *key; 
			struct Json_Type *value;
			struct Json_Type *next;
		} object;
		
		string_slice string;
		
		size_t integer;
		double floating_point;
	};
} Json_Type; 

// The following functions parse known types. 
// They handle their own errors, and return NULL 
// when an error has occored. 
internal Json_Type  *parse_key_value_pair(Parser *parser);
internal Json_Type       *parse_object(Parser *parser);
internal Json_Type       *parse_array(Parser *parser);
internal Json_Type       *parse_json(char *input_buffer);

internal b32              string_slice_to_integer(string_slice number, size_t *result);

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
}                                           \
    
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
            token_out->text_size = slice_buffer.size;
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
			// TODO(ziv): Fix the seemingly infinite call to the function as it creates a stackoverflow error.
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
    parser->location = old_location; // reseting the parser location.
	// because get_next_token moved the location, we use the old one instead.
    return success;
}

internal Json_Type *
parse_key_value_pair(Parser *parser)
{
	// looks at the first token in the key value pair.
    Token token;
    if (!get_next_token(parser, &token))  return NULL; // invalid token.
    
	// creating the key value pair as a Json_Type.
	Json_Type *kvp = (Json_Type *)context.alloc(sizeof(Json_Type));
	kvp->type = TYPE_Object; 
	kvp->object.next = NULL;
	
	//
	// checks for a key
	//
    if (token.token_type == TOKEN_STRING)
    {
        kvp->object.key = token.text;
    }
    else 
    {
        LogError("error parsing(%d:%d): inside a object expected a key of type 'STRING', got type '%s'.", 
                 parser->location.line, parser->location.character, token_name_map[token.token_type]); 
        return NULL;
    }
    
	//
	// check if there is a colon 
	//
    if (!get_next_token(parser, &token))  return NULL;
    if (token.token_type != TOKEN_COLON) 
    {
        LogError("error parsing(%d:%d): inside an object, after key '%s' expected COLON, got '%s'.", 
                 parser->location.line, parser->location.character, kvp->object.key, token.text);
        return NULL;
    }
	
    //
    // checks for a value
	//
	
	if (get_next_token(parser, &token))
	{
		// try to figure out the type.
		
		if (token.token_type == TOKEN_LEFT_CURLY)
		{
			kvp->object.value = parse_object(parser);
			return kvp;
		}
		else if (token.token_type == TOKEN_LEFT_BRACKET)
		{
			kvp->object.value = parse_array(parser);
			return kvp;
		}
		
		kvp->object.value = (Json_Type *)context.alloc(sizeof(Json_Type)); 
		Json_Type *value  = kvp->object.value;
		value->object.next = NULL;
		
		string_slice s; 
		s.data = token.text; 
		s.size = token.text_size;
		
		if (token.token_type == TOKEN_STRING) 
		{
			value->type = TYPE_String;
			value->string = s;
		}
		else if (token.token_type == TOKEN_INTEGER)
		{
			value->type = TYPE_Integer;
			if (!string_slice_to_integer(s, &value->integer))
			{
				// could not create an integer from the number. 
				// this probalby due to it being too big. 
			}
			
		}
		else if (token.token_type == TOKEN_FLOAT)
		{
			value->type = TYPE_Float; 
			value->floating_point = 0.4f; // some garbage value until I implement the conversion between string and floating point  
			
		}
		else 
		{
			LogError("error parsing(%d:%d): expected VALUE of type OBJECT/LIST/STRING/INTEGER/FLOAT got unknown value '%s'.", 
					 parser->location.line, parser->location.character, token.text);
			return NULL;
		}
		
	}
	return kvp;
}

internal Json_Type *
parse_object(Parser *parser)
{
    b32 success = false;
	Json_Type *node = NULL;
	
	Token token; 
	
	// empty object.
	if (peek_next_token(parser, &token) && 
		token.token_type == TOKEN_RIGHT_CURLY)
	{
		get_next_token(parser, &token); // eat the token
		return NULL;
	}
	
	
	Json_Type *kvl_head = parse_key_value_pair(parser);
	if (!kvl_head) return NULL; // error managed by parse_key_value_pair
	
	Json_Type *kvp = NULL;
    success = get_next_token(parser, &token);
	if (success && token.token_type == TOKEN_COMMA)
	{
		kvp = parse_key_value_pair(parser);
		kvl_head->object.next = kvp;
		
		success = get_next_token(parser, &token);
		while (kvp && success && token.token_type == TOKEN_COMMA)
		{
			kvp->object.next = parse_key_value_pair(parser);  
			kvp = kvp->object.next;
			
			success = get_next_token(parser, &token);
		}
	}
	else
	{
		// something
	}
	
	if (success) 
	{
		if (token.token_type == TOKEN_RIGHT_CURLY)
		{
			return kvl_head;
		}
		else
		{
			LogError("error parsing(%d:%d): expected right curly bracked, got '%s'.", 
					 parser->location.line, parser->location.character, token.text)
		}
	}
	
	
	// a problem while tokenizing happened.
	return NULL;
}

////////////////////////////////

internal b32 
array_add_element(Json_Type *arr, Json_Type elem)
{
	Assert(elem.type >= 0); // bad element.
	if (!arr->array.data)  return false;
	
	if (arr->array.capacity <= arr->array.index) 
	{
		// not enough space, realloc more space.
		// TODO(ziv): use realloc here. 
		size_t new_size = (size_t)((f64)arr->array.capacity * 1.5f); // TODO(ziv): find a better way of finding a better number.
		Json_Type *new_array = (Json_Type *)context.alloc(new_size);
		memcpy(new_array, arr->array.data, arr->array.capacity);
		arr->array.data = new_array; 
	}
	
	arr->array.data[arr->array.index++] = elem;
	return true;
}


////////////////////////////////

internal Json_Type *
parse_array(Parser *parser)
{
	Token token; 
    b32 success = false;
	
	
	// handle empty list.
	if (peek_next_token(parser, &token) && token.token_type == TOKEN_RIGHT_BRACKET)
	{
		get_next_token(parser, &token); // eat that token.
		return (Json_Type *)NULL;
	}
	
	
	// create the array
	Json_Type *arr = (Json_Type *)context.alloc(sizeof(Json_Type)); 
	arr->type = TYPE_Array; 
	arr->array.capacity = 4; 
	arr->array.index    = 0; 
	arr->array.data = (Json_Type *)context.alloc(sizeof(Json_Type) * arr->array.capacity);
	
	while (get_next_token(parser, &token)) 
	{
		
		if (token.token_type == TOKEN_LEFT_BRACKET)    // array 
		{
			Json_Type *temp = parse_array(parser); 
			if (!array_add_element(arr, *temp))
			{
				return NULL; // out of memeory. 
			}
			
		}
		else if (token.token_type == TOKEN_LEFT_CURLY) // object
		{
			Json_Type *temp = parse_object(parser);
			if (!array_add_element(arr, *temp))
			{
				return NULL; // out of memory.
			}
			
		}
		else if (token.token_type == TOKEN_STRING)     // string
		{
			Json_Type *jt_str = (Json_Type *)context.alloc(sizeof(Json_Type)); 
			jt_str->type = TYPE_String; 
			jt_str->string.data = token.text; 
			jt_str->string.size = token.text_size; 
			if (!array_add_element(arr, *jt_str))
			{
				return NULL; // out of memory.
			}
			
		}
		else if (token.token_type == TOKEN_INTEGER)    // integer 
		{
			Json_Type *jt_int = (Json_Type *)context.alloc(sizeof(Json_Type)); 
			jt_int->type = TYPE_Integer; 
			
			// create the string slice that stores the number info
			string_slice s = {0};
			s.size = token.text_size;
			s.data = token.text;
			
			if (!string_slice_to_integer(s, &jt_int->integer)) 
			{
				// unsuccessful
				LogError("error(%d:%d): internal error, could not convert a integer token to a integer", 
						 parser->location.line, parser->location.character);
				return NULL;
			}
			if (!array_add_element(arr, *jt_int))
			{
				return NULL; // out of memory
			}
			
		}
		else if (token.token_type == TOKEN_FLOAT)      // float
		{
			Json_Type *jt_float = (Json_Type *)context.alloc(sizeof(Json_Type)); 
			jt_float->type = TYPE_Integer; 
			
			// create the string slice that stores the number info
			string_slice s = {0};
			s.size = token.text_size;
			s.data = token.text;
			
			if (!string_slice_to_integer(s, &jt_float->integer)) 
			{
				// unsuccessful
				LogError("error(%d:%d): internal error, could not convert a integer token to a integer", 
						 parser->location.line, parser->location.character);
				return NULL;
			}
			if (!array_add_element(arr, *jt_float))
			{
				return NULL; // out of memory
			}
			
		}
		else if (token.token_type == TOKEN_RIGHT_BRACKET)
		{
			break;
		}
		else // this is an incorrect type
		{
			LogError("error(%d:%d): unknown type found %d.",
					 parser->location.line, parser->location.character, (int)token.token_type);  
			return NULL;
		}
		
		// Add the element to the list.
		if (peek_next_token(parser, &token) && token.token_type == TOKEN_COMMA) 
		{
			get_next_token(parser, &token); // go over the token
			//if (!array_add_element(jt_arr, jt)) { LogError("error: ran out of memory"); }
		}
		else if (arr->type == 0) // unknown token. Meaning before the comma there is no type.
		{
			
			LogError("error(%d:%d): missing comma when dividing between multiple types.", 
					 parser->location.line, parser->location.character); 
			return NULL;
		}
		
	}
	
	return arr; 
}

internal Json_Type *
parse_json(char *input_buffer)
{
	// setup
	init_token_to_string_map();
	init_global_context(4 * 1024);
	
	Parser parser = {0};
	parser.text   = input_buffer; 
	parser.location.line = 1; // Most code editors begin at line 1. 
	
	Json_Type *head = NULL; // head of the tree
	
	// look and see if it begins with a curly brace or a bracket.
	Token token = {0}; 
	b32 success = get_next_token(&parser, &token);
	if (token.token_type == TOKEN_LEFT_CURLY)
	{
		head = parse_object(&parser);
	}
	else if (token.token_type == TOKEN_LEFT_BRACKET)
	{
		head = parse_array(&parser);
	}
	else 
	{
		LogError("error parsing(%d:%d): did not find object/list type at the beginning", 
				 parser.location.line, parser.location.character);
	}
	
	// context.free();
	return head;
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



#if UINTPTR_MAX == 0xffffffff           /* 32-bit */
#define MAX_NUM_CHARACTER_LENGTH 10
#define JSON_INT_MAX INT32_MAX
#elif UINTPTR_MAX == 0xffffffffffffffff /* 64-bit */
#define MAX_NUM_CHARACTER_LENGTH 20
#define JSON_INT_MAX INT64_MAX
#else                                   /*  wtf   */
#endif

internal b32
string_slice_to_integer(string_slice number, size_t *result)
{
	Assert(number.data == null); // I expect to get valid data.
	if (!result || !number.data) return 0;
	
	size_t number_length = number.capacity;
	size_t pos = 1; // positions the numbers as they should. Used later.
	
	char *str = number.data;
	if (*str == '-')
	{
		pos           = -1; // make the number be negative (the pos will get multiplied with the number)
		number_length = number.capacity - 1; // because the character '-' is no longer taken into account
		str++; // skip the '-'
	}
	
	if (number_length > MAX_NUM_CHARACTER_LENGTH)  return 0; // character length is too large. 
	
	// TODO(ziv): use a function like pow? 
	u32 exp = number_length-1;
	for (u32 exp_counter = 0; exp_counter < exp; exp_counter++) { pos *= 10; } // 10^exp 
	
	size_t number_as_integer = 0; // the generated number
	if (number_length == MAX_NUM_CHARACTER_LENGTH) 
	{
		
		// 
		// Do checks for overflows after we compute almost all of the number
		// 
		
		pos /= 10; // we don't want the units.
		
		s32 temp_number = 0;
		s32 char_index  = 0;
		while((number.capacity-1) > char_index && 
			  *str && 
			  is_number(*str))
		{
			
			temp_number = to_number(*str);
			number_as_integer += temp_number * pos;
			
			pos /= 10; str++; char_index++;
		}
		
		if (number_as_integer > (JSON_INT_MAX / 10))  return 0; // there is an overflow. 
		
		if (number.capacity > char_index && *str && is_number(*str)) 
		{
			s32 units = to_number(*str);
			if (units > JSON_INT_MAX % 10)
			{
				return 0;
			}
			number_as_integer *= 10; 
			number_as_integer += units;
		}
	}
	else
	{
		s32 temp_number = 0;
		for (s32 char_index = 0; 
			 number.capacity > char_index && 
			 *str && is_number(*str); )
		{
			
			temp_number = to_number(*str);
			number_as_integer += temp_number * pos;
			pos /= 10;
			
			str++; char_index++;
		}
	}
	
	*result = number_as_integer;
	return true;
}


#endif //JSON_HL_H

