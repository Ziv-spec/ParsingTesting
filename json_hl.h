/* date = February 21st 2021 11:16 pm */

#ifndef JSON_HL_H
#define JSON_HL_H

// TODO(ziv):
// Fix the memory issues regarding the arrays. 
// When I do array inside a array, a memory corruption
// happens every time that I do so. 
//
// Add support for booleans as they are a standart type that I should support.
//
// make the arrays expan as they currenly do not.
// fix the context please as it is currently not functioning correctly. 
// 
// Features: 
//   *have a hash table like access (e.g. a function that creates a hash table from the json tree). 
//   *have a function that allows access like the feature above but it will not truly as fast (linear search)
//   *maybe use more/less of the C standard library.
//   *add more error messages and better ones (or rethink the who structure tho I don't recomment) 

// TODO(ziv): To make this a true single header library 
// it is required to change the types to different names 
// to avoid naming colissions with others projects. 
// But this is last on my priority list.

//
// Types
// 

#include <stdint.h>
typedef int32_t  s32;
typedef int64_t  s64;

typedef uint8_t  u8; 
typedef uint32_t u32;
typedef uint64_t u64; // NOTE(ziv): not used

typedef float    f32;
typedef double   f64;

// booleans 
typedef int32_t  b32;
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


#include <string.h>
#include <stdio.h> 
#include <stdlib.h>


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
	size_t buffer_capacity;    // size of the memory buffer.
	size_t offset;             // where the last allocation happened.
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
	
	// not enough space in the buffer, there is a need for a new allocation. 
	
	// NOTE(ziv): probably can make this one allocation instead of two. 
	Memory_Block *memory_block = (Memory_Block *)malloc(sizeof(Memory_Block)); 
	memory_block->next = NULL; 
	memory_block->buffer_capacity = context.allocation_size; 
	memory_block->offset = requested_size; 
	memory_block->buffer = malloc(context.allocation_size);
	if (memory_block && memory_block->buffer)
	{
		context.memory.tail->next = memory_block;
		context.memory.tail = context.memory.tail->next;
		return memory_block->buffer;
	}
	return NULL;
}

internal void
json_memory_free()
{
	Memory_Block *block = context.memory.head;
	Memory_Block *next_block = block->next;
	
	while (block->next)
	{
		next_block = block->next;
		free(block->buffer);
		free(block); 
		block = next_block;
	}
	if (block)
	{
		free(block->buffer);
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
	context.memory.head->next = NULL;
	context.memory.head->offset = 0;
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
	// all of tokens that I currently support, 
	// maped to a string represeting it. 
    
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
			string_slice key; 
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
internal Json_Type *parse_key_value_pair(Parser *parser);
internal b32 parse_object(Parser *parser, Json_Type *jt_object);
internal b32 parse_array(Parser *parser,  Json_Type *jt_array);
internal Json_Type *parse_json(char *input_buffer);

internal b32 string_slice_to_integer(string_slice number, size_t *result);
internal b32 string_slice_to_float(string_slice number, double *result);

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
			token_out->text_size = slice_buffer.size;
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
					token_out->text_size = slice_buffer.size; 
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
            // IMPORTANT(ziv): DO THIS PLEASE !!! 
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
	
	// checks if there is a key
    if (token.token_type != TOKEN_STRING)
    {
        LogError("error parsing(%d:%d): inside a object expected a key of type 'STRING', got type '%s'.", 
                 parser->location.line, parser->location.character, token_name_map[token.token_type]); 
        return NULL;
	}
	// it exists
	string_slice s; 
	s.data = token.text; 
	s.size = token.text_size;
	kvp->object.key = s; 
	
	// check if there is a colon 
    if (!get_next_token(parser, &token))  return NULL;
    if (token.token_type != TOKEN_COLON) 
    {
        LogError("error parsing(%d:%d): inside an object, after key '%s' expected COLON, got '%s'.", 
                 parser->location.line, parser->location.character, kvp->object.key.data, token.text);
        return NULL;
    }
	
    // checks for a value
	if (get_next_token(parser, &token))
	{
		kvp->object.value = (Json_Type *)context.alloc(sizeof(Json_Type));
		if (token.token_type == TOKEN_LEFT_CURLY)       // object
		{
			kvp->object.value->object.value = NULL; 
			kvp->object.value->object.next  = NULL;
			if (parse_object(parser, kvp->object.value))
				return kvp;
			return NULL;
		}
		else if (token.token_type == TOKEN_LEFT_BRACKET) // array
		{
			kvp->object.value->array.data     = NULL; 
			kvp->object.value->array.capacity = 0;
			kvp->object.value->array.index    = 0;
			if (parse_array(parser, kvp->object.value)) 
				return kvp;
			return NULL;
		}
		
		// simple value
		Json_Type *value   = kvp->object.value;
		value->object.next = NULL;
		
		string_slice s; 
		s.data = token.text; 
		s.size = token.text_size;
		
		if (token.token_type == TOKEN_STRING) 
		{
			value->type   = TYPE_String;
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
			string_slice s; 
			s.data = token.text; 
			s.size = token.text_size; 
			if (!string_slice_to_float(s, &value->floating_point))
			{
				return NULL;
			}
			
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

internal b32
parse_object(Parser *parser, Json_Type *jt_object)
{
	b32 success = false;
	Token token; 
	
	// empty object.
	if (peek_next_token(parser, &token) && 
		token.token_type == TOKEN_RIGHT_CURLY)
	{
		get_next_token(parser, &token); // eat the token
		return false;
	}
	
	Json_Type *kvl_head = parse_key_value_pair(parser);
	if (!kvl_head) return false; 
	
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
			*jt_object = *kvl_head; 
			return true;
		}
		else
		{
			LogError("error parsing(%d:%d): expected right curly bracked, got '%s'.", 
					 parser->location.line, parser->location.character, token.text)
		}
	}
	
	return false;
}

////////////////////////////////

internal b32 
array_add_element(Json_Type *arr, Json_Type elem)
{
	Assert(arr->type == TYPE_Array);
	Assert(elem.type > 0); // bad element.
	if (!arr->array.data)  return false;
	
	if (arr->array.capacity <= arr->array.index) 
	{
		// not enough space, realloc more space.
		// TODO(ziv): use realloc here. 
		size_t new_size = (size_t)((f64)arr->array.capacity		* 1.5f); // TODO(ziv): find a better way of finding a better number.
		Json_Type *new_array = (Json_Type *)context.alloc(new_size);
		memcpy(new_array, arr->array.data, arr->array.capacity);
		arr->array.data = new_array; 
	}
	
	arr->array.data[arr->array.index++] = elem;
	return true;
}

////////////////////////////////

internal b32
parse_array(Parser *parser, Json_Type *jt_array)
{
	Token token; 
    b32 success = false;
	
	// handle empty list.
	if (peek_next_token(parser, &token) && token.token_type == TOKEN_RIGHT_BRACKET)
	{
		get_next_token(parser, &token); // eat that token.
		return false;
	}
	
	// create the array
	jt_array->type = TYPE_Array; 
	jt_array->array.capacity = 50; 
	jt_array->array.index    = 0; 
	jt_array->array.data = (Json_Type *)context.alloc(sizeof(Json_Type) * jt_array->array.capacity);
	
	while (get_next_token(parser, &token)) 
	{
		
		if (token.token_type == TOKEN_LEFT_BRACKET)    // array 
		{
			Json_Type temp_arr = {0};
			if (!parse_array(parser, &temp_arr))        return false; // could not parse the array
			if (!array_add_element(jt_array, temp_arr)) return false; // out of memeory.
			
		}
		else if (token.token_type == TOKEN_LEFT_CURLY) // object
		{
			Json_Type temp_object = {0};
			if (!parse_object(parser, &temp_object))       return false; // could not parse the object
			if (!array_add_element(jt_array, temp_object)) return false; // out of memory.
			
		}
		else if (token.token_type == TOKEN_STRING)     // string
		{
			Json_Type jt_str = {0};
			jt_str.type = TYPE_String; 
			jt_str.string.data = token.text; 
			jt_str.string.size = token.text_size; 
			if (!array_add_element(jt_array, jt_str)) return false; // out of memory.
			
		}
		else if (token.token_type == TOKEN_INTEGER)    // integer 
		{
			Json_Type jt_int = {0};
			jt_int.type = TYPE_Integer; 
			
			// create the string slice that stores the number info
			string_slice s = {0};
			s.size = token.text_size;
			s.data = token.text;
			if (!string_slice_to_integer(s, &jt_int.integer)) 
			{
				LogError("error(%d:%d): internal error, could not convert a integer token to a integer", 
						 parser->location.line, parser->location.character);
				return NULL; // unable to convert to integer
			}
			
			if (!array_add_element(jt_array, jt_int)) return false; // out of memory
			
		}
		else if (token.token_type == TOKEN_FLOAT)      // float
		{
			Json_Type jt_float = {0}; 
			jt_float.type = TYPE_Float; 
			
			// create the string slice that stores the number info
			string_slice s = {0};
			s.size = token.text_size;
			s.data = token.text;
			if (!string_slice_to_float(s, &jt_float.floating_point)) 
			{
				// unsuccessful
				LogError("error(%d:%d): internal error, could not convert a integer token to a integer", 
						 parser->location.line, parser->location.character);
				return false;
			}
			
			if (!array_add_element(jt_array, jt_float)) return false; // out of memory
			
		}
		else if (token.token_type == TOKEN_RIGHT_BRACKET) 
		{
			break;
		}
		else // this is an incorrect type
		{
			LogError("error(%d:%d): unknown type found %d.",
					 parser->location.line, parser->location.character, (int)token.token_type);  
			return false;
		}
		
		if (peek_next_token(parser, &token) && token.token_type == TOKEN_COMMA) 
		{
			get_next_token(parser, &token); // go over the token
		}
		else if (!(token.token_type & (TOKEN_RIGHT_BRACKET | TOKEN_RIGHT_CURLY | 
									   TOKEN_FLOAT | TOKEN_INTEGER | TOKEN_STRING)))
		{
			
			LogError("error(%d:%d): missing comma when dividing between multiple types.", 
					 parser->location.line, parser->location.character); 
			return false;
		}
		
	}
	
	return true; 
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
	
	Json_Type *head = (Json_Type *)context.alloc(sizeof(Json_Type));
	
	// look and see if it begins with a curly brace or a bracket.
	Token token = {0}; 
	b32 success = get_next_token(&parser, &token);
	if (token.token_type == TOKEN_LEFT_CURLY)
	{
		if (!parse_object(&parser, head)) return NULL;
	}
	else if (token.token_type == TOKEN_LEFT_BRACKET)
	{
		if (!parse_array(&parser, head)) return NULL;
	}
	else 
	{
		LogError("error parsing(%d:%d): did not find object/list type at the beginning", 
				 parser.location.line, parser.location.character);
	}
	
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


internal b32
string_slice_to_integer(string number, size_t *result)
{
	Assert(result || number.data || number.size > 1); // I expect to get valid data.
	
	unsigned long long ival= 0 , i = 0, oval; 
	u8 n = 1, c = number.data[i]; 
	
	if (c == '-' || c == '+') 
	{ 
		n = ((c == '-') ? -1:1);
		i++;
	} 
	
	u32 index = 0;
	while ((c = number.data[i++]) && index < number.size)
	{
		if (!is_number(c)) {
			LogError("error integer");
			return false; 
		}
		ival = (ival * 10) + (c - '0'); 
		if ((n > 0 && ival > (size_t)-1)
			|| (n < 0 && ival > (1 << 31)))
		{
			LogError("error integer");
			errno = ERANGE;  // TODO(ziv): check what is this doing.
			*result = (n > 0 ? LONG_MAX : LONG_MIN); 
			return false;
		}
		index++;
	}
	*result = (n > 0 ? (long)ival : -(long)ival); 
	return true;
}

internal b32 
string_slice_to_float(string_slice number, double *result)
{
	Assert(result || number.data || number.size > 1); 
	
	u32 sign = 1; 
	
	string s; 
	s.data  = number.data; 
	s.index = 0; 
	
	// get the non-decimal number
	u32 i = 0;
	while (i < number.size && number.data[i] != '.') i++; 
	s.index = i; 
	size_t num; 
	if (!string_slice_to_integer(s, &num)) return false;
	
	// get the decimal number
	s.data = s.data + i + 1; 
	s32 decimal_size = s.index;
	i = s.index; 
	while (i < number.size) i++;
	s.index = i; 
	size_t decimal; 
	if (!string_slice_to_integer(s, &decimal)) return false; 
	decimal_size = i - decimal_size; 
	
	// combine to a number
	double pow = 1; 
	for (u32 index = 0; index < decimal_size-1; index++) pow *= 10.0;
	*result = (double)num + ((double)decimal/(double)pow);
	return true;
}


#endif //JSON_HL_H
