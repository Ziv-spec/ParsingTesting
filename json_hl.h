// json_hl.h 
// authored by Ziv 2020-2021
//
// This libaray parses json files. 
// 
// TODO: 
//     Handle integers properly such that they don't overflow 
//     clean up tokens like TOKEN_NONE
//     maybe have a key_value_pair representation in the Json_Type
//     have a hash table like access? 
//     function to access types not as hash table? 
//     maybe use more/less of the C standard library?
//
// USAGE
// 
// To use this library you should include it to your project
// as per usual. 
//
//   jhl_parse_json()  -- parse the text buffer and return the json structure
//   
// To use the json structure given from it do the following: 
// first read the type to figure out what kind of information 
// does it hold in it. Then, access the type. 
//
// -- The structure --  
//
// struct Json_Type
// {
//   Simple_Types type;
//   union 
//   {
//     struct {
//       size_t capacity;
//       size_t index;
//       struct Json_Type *data;
//     } array;
// 
//     struct {
//       jhl_string key; 
//       struct Json_Type *value;
//       struct Json_Type *next;
//     } object;
//   
//     jhl_string string;
//     size_t integer;
//     double floating_point;
//     b32 boolean;
//   };
// }; 
//

////////////////////////////////
//
// Sample Program
//

#if 0
#include "json_hl.h"  // The header library.

// some really quick witted debug printing for the Json_Type 
// which can be changed in any way that you would like.

JHL_STATIC void 
debug_print_padding(int padding_level)
{
	for (int padding_counter = 0; padding_counter < padding_level; padding_counter++)
	{
		JHL_Log("  ");
	}
}

JHL_STATIC void debug_print_object(Json_Type *object, int padding_level);
JHL_STATIC void debug_print_array(Json_Type *arr, int padding_level);

JHL_STATIC void
debug_print_value(Json_Type *value, int padding_level)
{
	
	if (value->type == TYPE_Object)
	{
		if (value)  debug_print_object(value, padding_level);
		else JHL_Log("{}");
	}
	else 
	{
		if (value->type == TYPE_String)
		{
			JHL_Log("%c%s%c", '"',(char *)jhl_slice_to_string(value->string), '"'); 
		}
		else if (value->type == TYPE_Integer)
		{
			JHL_Log("%zu", value->integer);
		}
		else if (value->type == TYPE_Float)
		{
			JHL_Log("%lf", value->floating_point);
		}
		else if (value->type == TYPE_Object)
		{
			debug_print_object(value, padding_level + 1);
		}
		else if (value->type == TYPE_Array)
		{
			debug_print_array(value, padding_level);
		}
		else if (value->type == TYPE_Bool)
		{
			if (value->boolean) 
			{
				JHL_Log("true");
			}
			else
			{
				JHL_Log("false");
			}
		}
	}
}

JHL_STATIC void
debug_print_key_value_pair(jhl_string key, Json_Type *value, int padding_level)
{
	// print the key
	debug_print_padding(padding_level); JHL_Log("%c%s%c",'"', key.data, '"');  
	
	// print the value
	JHL_Log(": "); debug_print_value(value, padding_level); 
}


JHL_STATIC void
debug_print_object(Json_Type *object, int padding_level)
{
	Assert(object->type == TYPE_Object);
	if (!object) return;
	
	JHL_Log("{\n"); 
	
	for (; object->object.next; object = object->object.next)
	{
		debug_print_key_value_pair(object->object.key,
								   object->object.value, 
								   padding_level + 1); JHL_Log(", \n");
	}
	debug_print_key_value_pair(object->object.key, 
							   object->object.value, 
							   padding_level + 1); JHL_Log("\n");
	
	debug_print_padding(padding_level); JHL_Log("}");
}

JHL_STATIC void
debug_print_array(Json_Type *arr, int padding_level)
{
	Assert(arr->type == TYPE_Array);
	
	JHL_Log("[\n"); 
	if (arr)
	{
		debug_print_padding(padding_level + 1); 
		
		int i = 0;
		for (; i < arr->array.index-1; i++)
		{
			Simple_Types type = arr->array.data[i].type;
			
			if (type == TYPE_String)
			{
				JHL_Log("%c%s%c", '"',(char *)jhl_slice_to_string(arr->array.data[i].string), '"'); 
				JHL_Log(",\n"); debug_print_padding(padding_level+1);
			}
			else if (type == TYPE_Integer)
			{
				JHL_Log("%zu", arr->array.data[i].integer);
				JHL_Log(",\n"); debug_print_padding(padding_level+1);
			}
			else if (type == TYPE_Float)
			{
				JHL_Log("%lf", arr->array.data[i].floating_point);
				JHL_Log(",\n"); debug_print_padding(padding_level+1);
			}
			else if (type == TYPE_Object)
			{
				debug_print_object(arr->array.data+i, padding_level + 1);
				JHL_Log(","); JHL_Log("\n"); debug_print_padding(padding_level + 1); 
			}
			else if (type == TYPE_Array)
			{
				debug_print_array(arr->array.data+i, padding_level + 1);
				JHL_Log(","); JHL_Log("\n"); debug_print_padding(padding_level + 1); 
			}
		}
		
		// last element without the ',' 
		Simple_Types type = arr->array.data[i].type;
		if (type == TYPE_String)
		{
			JHL_Log("%c%s%c", '"',(char *)jhl_slice_to_string(arr->array.data[i].string), '"'); 
		}
		else if (type == TYPE_Integer)
		{
			JHL_Log("%zu", arr->array.data[i].integer);
		}
		else if (type == TYPE_Float)
		{
			JHL_Log("%lf", arr->array.data[i].floating_point);
		}
		else if (type == TYPE_Object)
		{
			debug_print_object(arr->array.data+i, padding_level + 1);
		}
		else if (type == TYPE_Array)
		{
			debug_print_array(arr->array.data+i, padding_level +1);
		}
		
	}
	
	JHL_Log("\n"); debug_print_padding(padding_level); JHL_Log("]"); 
}

JHL_STATIC void 
debug_print_json_tree(Json_Type *json_type)
{
	if (json_type)
	{
		if (json_type->type == TYPE_Object)
		{
			debug_print_object(json_type, 0);
		}
		else if (json_type->type == TYPE_Array)
		{
			debug_print_array(json_type, 0);
		}
		JHL_Log("\n");
	}
}

JHL_STATIC void
debug_print_token(Token token)
{
    
    char *token_name = token_name_map[token.token_type];
    
    char padding[255];
    int longest_size = jhl_string_size("TOKEN_RIGHT_BRACKET") - 1;
    int token_string_size = jhl_string_size(token_name);
    int counter = 0;
    for (; counter < longest_size - token_string_size; counter++)
    {
        padding[counter] = ' ';
    }
    padding[counter] = '\0';
    
    printf("%s%s%d:%d  %s\n", token_name,padding, token.location.line, token.location.character, token.text); 
}



int main(int argc, char **argv)
{
#if DEBUG
	if (argc == 1)
#else
    if (argc == 2) 
#endif 
    {
        char *file_name = argv[1]; 
		
#if DEBUG
        file_name = "C:\\dev\\json\\json_example.json";
#endif
		
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
                buffer[file_size] = '\0'; // null terminate the file.
                
				// prints out the file contents.
                fprintf(stdout, "Json data:\n%s\n\n", buffer);
                
                fclose(file);
                
				Json_Type *json_tree = jhl_parse_json(buffer);
				// print the structure (visual representation)
				debug_print_json_tree(json_tree);
				
				// although, it should get freed at the end of the application.
				// so... there is no need to do this in here. 
				free(buffer);
				
				jhl_mem_free(context); // free the json structure as one block
            }
            else
            {
                fprintf(stderr, "Error: Cannot allocate space for file buffer.\n"); 
            }
        }
        else
        {
            fprintf(stderr, "Error: Can not open file '%s'.\n", file_name); 
            
        }
        
    }
    else
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]); 
    }
    return 0;
}
#endif /* example program */




#ifndef JSON_HL_H
#define JSON_HL_H

////////////////////////////////
//
// Types
// 

#include <stdint.h>
typedef unsigned char u8; 
typedef unsigned int  b32; // boolean

#define false 0
#define true 1

#if DEBUG
#define Assert(expression) if(!(expression)) { *(int *)NULL = 0; }
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
#ifndef JHL_STATIC
#define JHL_STATIC
#else
#undef JHL_STATIC 
#define JHL_STATIC static
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
#include <limits.h>

////////////////////////////////
//
// Logging
// 

JHL_STATIC b32 error_showed; // a global variable that allows or disables output of errors. 
// currently this is a patch for a internal problem, although, it should provide a good method 
// for reuse of code (that exists) and not needing to see the error messages (if I want to do 
// special checks so I could understand the following better).
#define JHL_Log(...) fprintf(stdout, __VA_ARGS__); 
#define JHL_LogError(...) if(!error_showed) { fprintf(stderr, __VA_ARGS__); fprintf(stderr, " %d", __LINE__); fprintf(stderr, "\n"); error_showed = true;}


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

typedef struct JHL_Memory_Block
{
	struct JHL_Memory_Block *next; // next memory pool.
	size_t buffer_capacity;    // size of the memory buffer.
	size_t offset;             // where the last allocation happened.
	void   *buffer;            // memory buffer. 
} JHL_Memory_Block;

typedef struct JHL_Memory_Pool
{
	JHL_Memory_Block *head; 
	JHL_Memory_Block *tail; 
} JHL_Memory_Pool;

typedef struct JHL_Context
{
	JHL_Memory_Pool memory; // the default memory pool
	int allocation_size;    // the default size for each block of memory.
} JHL_Context; 

global JHL_Context context;

//JHL_STATIC void init_global_context(int pool_size); 

JHL_STATIC void *
jhl_mem_alloc(JHL_Context context, int requested_size)
{
	JHL_Memory_Block *last_block = context.memory.tail;
	
	// check if there is enough space in the block
	if ((last_block->offset + requested_size) < last_block->buffer_capacity) 
	{
		void *requested_memory = (u8 *)last_block->buffer + last_block->offset;
		last_block->offset += requested_size; 
		return requested_memory;
	}
	
	// not enough space in the buffer, there is a need for a new allocation. 
	
	JHL_Memory_Block *memory_block = (JHL_Memory_Block *)malloc(sizeof(JHL_Memory_Block) + 
																context.allocation_size); 
	memory_block->next = NULL; 
	memory_block->buffer_capacity = context.allocation_size; 
	memory_block->offset = requested_size; 
	memory_block->buffer = (u8 *)memory_block + context.allocation_size;
	if (memory_block && memory_block->buffer)
	{
		context.memory.tail->next = memory_block;
		context.memory.tail = context.memory.tail->next;
		return memory_block->buffer;
	}
	return NULL;
}

JHL_STATIC void
jhl_mem_free(JHL_Context context)
{
	JHL_Memory_Block *block = context.memory.head;
	JHL_Memory_Block *next_block = block->next;
	
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

JHL_STATIC void 
jhl_init_global_context(int block_size)
{ 
	context.allocation_size = block_size;
	int total_allocation_size = sizeof(JHL_Memory_Block) + context.allocation_size;
	context.memory.head = (JHL_Memory_Block *)malloc(total_allocation_size);
	context.memory.head->next = NULL;
	context.memory.head->offset = 0;
	context.memory.head->buffer_capacity = context.allocation_size;
	context.memory.head->buffer = (u8 *)context.memory.head + sizeof(JHL_Memory_Block); 
	context.memory.tail = context.memory.head;
	
}


////////////////////////////////
// 
// Strings
// 

typedef struct jhl_string jhl_string;
struct jhl_string
{
    union
    {
        int size;
        int index;
		int capacity; 
    };
    
	char *data;
};

JHL_STATIC int   jhl_string_size(char *string);         /* returns the nullterminated strings size */
JHL_STATIC char *jhl_slice_to_string(jhl_string slice); /* converts a slice type to a null terminated character array */

JHL_STATIC b32 jhl_string_to_integer(jhl_string number, size_t *result); 
JHL_STATIC b32 jhl_string_to_float(jhl_string number, double *result);

JHL_STATIC int 
jhl_string_size(char *string)
{
	int string_size = 0; 
	while (*string)
	{
		string_size++;
		string++;
	}
	return string_size;
}

JHL_STATIC char *
jhl_slice_to_string(jhl_string slice)
{
	
	char *result = (char *)jhl_mem_alloc(context, (slice.size+1)); 
	char *temp_char = result; 
	for (int counter = 0; counter < slice.size; counter++)
	{
		*temp_char++ = *slice.data++;
	}
	*temp_char = '\0';
	return result;
}


JHL_STATIC b32
jhl_string_to_integer(jhl_string number, size_t *result)
{
	Assert(result || number.data || number.size > 1); // I expect to get valid data.
	
	unsigned long long ival= 0 , i = 0, oval; 
	u8 n = 1, c = number.data[i]; 
	
	if (c == '-' || c == '+') 
	{ 
		n = ((c == '-') ? -1:1);
		i++;
	} 
	
	unsigned int index = 0;
	while ((c = number.data[i++]) && index < number.size)
	{
		if (!is_number(c)) {
			JHL_LogError("error integer");
			return false; 
		}
		ival = (ival * 10) + (c - '0'); 
		if ((n > 0 && ival > (size_t)-1)
			|| (n < 0 && ival > (1 << 31)))
		{
			JHL_LogError("error integer");
			//errno = ERANGE;  // TODO(ziv): check what is this doing.
			*result = (n > 0 ? LONG_MAX : LONG_MIN); 
			return false;
		}
		index++;
	}
	*result = (n > 0 ? (long)ival : -(long)ival); 
	return true;
}

JHL_STATIC b32 
jhl_string_to_float(jhl_string number, double *result)
{
	Assert(result || number.data || number.size > 1); 
	
	unsigned int sign = 1; 
	
	jhl_string s; 
	s.data  = number.data; 
	s.index = 0; 
	
	// get the non-decimal number
	unsigned int i = 0;
	while (i < number.size && number.data[i] != '.') i++; 
	s.index = i; 
	size_t num; 
	if (!jhl_string_to_integer(s, &num)) return false;
	
	// get the decimal number
	s.data = s.data + i + 1; 
	int decimal_size = s.index;
	i = s.index; 
	while (i < number.size) i++;
	s.index = i; 
	size_t decimal; 
	if (!jhl_string_to_integer(s, &decimal)) return false; 
	decimal_size = i - decimal_size; 
	
	// combine to a number
	double pow = 1; 
	for (unsigned int index = 0; index < decimal_size-1; index++) pow *= 10.0;
	*result = (double)num + ((double)decimal/(double)pow);
	return true;
}

////////////////////////////////
// 
// Lexing
// 

typedef struct Location
{
    int index;     // index withing the general buffer.
    int line;      // line number withing the file.
    int character; // character at the line.
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
    TOKEN_BOOL          = 1024,// 40x0
	TOKEN_NONE          = 2048 // 80x0
} Token_Types;

typedef struct Token
{
    Token_Types token_type; // Type of the token.
    Location location;      // location of the token in the text buffer (inside the parser).
    char *text;   // text for the token.
	int text_size;// the size of the text.
} Token;

#define TOKEN_COUNT 13 // Only for the map down below.
static char *token_name_map[TOKEN_COUNT]; 

JHL_STATIC void
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
	token_name_map[TOKEN_BOOL]          = "BOOL";
}


JHL_STATIC b32 jhl_get_next_token(Parser *parser,    // advances the parsers location.
								  Token *token_out); // the token will get put here.
// If there is no token (or it is unknown), it returns 0.

JHL_STATIC b32 jhl_peek_next_token(Parser *parser,    // does not advance the parsers location. 
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
	TYPE_Bool    = 32,
	TYPE_Null    = 64
} Simple_Types;

typedef struct Json_Type Json_Type; 
struct Json_Type
{
	Simple_Types type;
	
	union 
	{
		struct {
			size_t capacity;
			size_t index;
			struct Json_Type *data;
		} array;
		
		struct {
			jhl_string key; 
			struct Json_Type *value;
			struct Json_Type *next;
		} object;
		
		jhl_string string;
		
		size_t integer;
		
		double floating_point;
		
		b32 boolean;
	};
}; 

// The following functions parse known types. 
// They handle their own errors, and return NULL 
// when an error has occored. 
JHL_STATIC Json_Type *jhl_parse_key_value_pair(Parser *parser);
JHL_STATIC b32        jhl_parse_object(Parser *parser, Json_Type *jt_object);
JHL_STATIC b32        jhl_parse_array(Parser *parser,  Json_Type *jt_array);
JHL_STATIC Json_Type *parse_json(char *input_buffer);

////////////////////////////////

JHL_STATIC b32
jhl_get_next_token(Parser *parser, Token *token_out)
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
        jhl_string slice_buffer = {0}; 
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
			JHL_LogError("error tokenizing(%d:%d): expected closing semi colon for a STRING type.", 
						 parser->location.line, parser->location.character);
		}
		else if (*cursor)
        {
            parser->location.index++; // skip the ending "
			parser->location.character++;
            
            char *token_text = jhl_slice_to_string(slice_buffer);
            token_out->text_size = slice_buffer.size;
			token_out->text = token_text;
            token_out->token_type = TOKEN_STRING;
            success = true;
        }
        else // the cursor is a nullterminator aka end of file.  
        {
            JHL_LogError("error tokenizing(%d:%d): end of file.\nThis is probably due to a missing semi colon.", 
						 parser->location.line, parser->location.character);
            return false;
        }
        
    }
    else if (is_number(*cursor)) // TOKEN_INTEGER or TOKEN_FLOAT
    {
        jhl_string slice_buffer = {0};
        slice_buffer.data = cursor; // beginning of slice.
        
        // Loops until it finds something that is not a number.
        while (is_number(*cursor))
        {
            slice_buffer.size++;
            parser->location.index++;
			parser->location.character++;
        }
        
        if (*cursor == '.') 
        {
            parser->location.index++; // skip the '.'
            parser->location.character++;
			slice_buffer.size++;
            
            while (is_number(*cursor))
            {
                slice_buffer.size++;
                parser->location.index++;
				parser->location.character++;
            }
            token_out->text = jhl_slice_to_string(slice_buffer);
			token_out->text_size = slice_buffer.size;
            token_out->token_type = TOKEN_FLOAT;
            success = true;
        }
        else // got unexpected characters after the numbers
        {
			Token next_token;
            if (jhl_peek_next_token(parser, &next_token))
            {
				if (next_token.token_type == TOKEN_UNKNOWN)
				{
					JHL_LogError("error(%d:%d): unknown value", 
								 parser->location.line, parser->location.character);
					
					return false;
				}
				token_out->text = slice_buffer.data; 
				token_out->text_size = slice_buffer.size; 
				token_out->token_type = TOKEN_INTEGER;
				success = true;
			}
        }
    }
	else if (*cursor == 'f' || *cursor == 't')
	{
		if (*cursor == 'f') // expecting "false"
		{
			char *boolean_false = "false";
			int boolean_false_size = jhl_string_size(boolean_false); 
			int index = 0;
			
			// check if boolean false
			while (boolean_false[index] && *cursor && 
				   boolean_false[index] == *cursor)
			{
				parser->location.index++;
				parser->location.character++; 
				index++;
			}
			if (index < boolean_false_size) 
			{
				JHL_LogError("error(%d:%d): unrecognized token begins with '%c'", 
							 parser->location.line, parser->location.character, 
							 parser->text[parser->location.index-1]); 
				return false;
			}
			
			token_out->text      = boolean_false; 
			token_out->text_size = boolean_false_size; 
		}
		else if (*cursor = 't') // "expecting "true"
		{
			char *boolean_true = "true";
			int boolean_true_size = jhl_string_size(boolean_true); 
			int index = 0;
			
			// check if boolean true
			while (boolean_true[index] && *cursor && 
				   boolean_true[index] == *cursor)
			{
				parser->location.index++;
				parser->location.character++;
				index++;
			}
			if (index < boolean_true_size) 
			{
				JHL_LogError("error(%d:%d): unrecognized token begins with '%c'", 
							 parser->location.line, parser->location.character, 
							 parser->text[parser->location.index-1]); 
				return false;
			}
			
			token_out->text      = boolean_true; 
			token_out->text_size = boolean_true_size; 
		}
		
		token_out->token_type = TOKEN_BOOL;
		success = true;
		
	}
    else if (*cursor == '\0')
    {
        token_out->token_type = TOKEN_NONE;
        token_out->text = '\0';
        return false;
    }
    else // TOKEN_UNKNOWN 
	{ 
		JHL_LogError("error(%d:%d): unrecognized token begins with '%c'", 
					 parser->location.line, parser->location.character, 
					 parser->text[parser->location.index]); 
	}
    
    // synces the out token location.  
    token_out->location = parser->location; 
    
    return success;
}

JHL_STATIC b32
jhl_peek_next_token(Parser *parser, Token *token_out)
{
    Location old_location = parser->location;
    b32 success = jhl_get_next_token(parser, token_out); 
    parser->location = old_location; // reseting the parser location.
	// because jhl_get_next_token moved the location, we use the old one instead.
    return success;
}

JHL_STATIC Json_Type *
jhl_parse_key_value_pair(Parser *parser)
{
	// looks at the first token in the key value pair.
    Token token;
    if (!jhl_get_next_token(parser, &token))  return NULL; // invalid token.
    
	// creating the key value pair as a Json_Type.
	Json_Type *kvp = (Json_Type *)jhl_mem_alloc(context, (sizeof(Json_Type)));
	kvp->type = TYPE_Object; 
	kvp->object.next = NULL;
	
	// checks if there is a key
	if (token.token_type != TOKEN_STRING)
	{
		JHL_LogError("error parsing(%d:%d): inside a object expected a key of type 'STRING', got type '%s'.", 
					 parser->location.line, parser->location.character, token_name_map[token.token_type]); 
		return NULL;
	}
	// it exists
	jhl_string s; 
	s.data = token.text; 
	s.size = token.text_size;
	kvp->object.key = s; 
	
	// check if there is a colon 
	if (!jhl_get_next_token(parser, &token))  return NULL;
	if (token.token_type != TOKEN_COLON) 
	{
		JHL_LogError("error parsing(%d:%d): inside an object, after key '%s' expected COLON, got '%s'.", 
					 parser->location.line, parser->location.character, kvp->object.key.data, token.text);
		return NULL;
	}
	
	// checks for a value
	if (jhl_get_next_token(parser, &token))
	{
		kvp->object.value = (Json_Type *)jhl_mem_alloc(context, (sizeof(Json_Type)));
		if (token.token_type == TOKEN_LEFT_CURLY)       // object
		{
			kvp->object.value->object.value = NULL; 
			kvp->object.value->object.next  = NULL;
			if (jhl_parse_object(parser, kvp->object.value))
				return kvp;
			return NULL;
		}
		else if (token.token_type == TOKEN_LEFT_BRACKET) // array
		{
			kvp->object.value->array.data     = NULL; 
			kvp->object.value->array.capacity = 0;
			kvp->object.value->array.index    = 0;
			if (jhl_parse_array(parser, kvp->object.value)) 
				return kvp;
			return NULL;
		}
		
		// simple value
		Json_Type *value   = kvp->object.value;
		value->object.next = NULL;
		
		jhl_string s; 
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
			if (!jhl_string_to_integer(s, &value->integer))
			{
				// could not create an integer from the number. 
				// this probalby due to it being too big. 
			}
			
		}
		else if (token.token_type == TOKEN_FLOAT)
		{
			value->type = TYPE_Float;
			jhl_string s; 
			s.data = token.text; 
			s.size = token.text_size; 
			if (!jhl_string_to_float(s, &value->floating_point))
			{
				return NULL;
			}
			
		}
		else if (token.token_type == TOKEN_BOOL)
		{
			value->type = TYPE_Bool; 
			value->boolean = token.text[0] == 't';
			
		}
		else 
		{
			JHL_LogError("error parsing(%d:%d): expected VALUE of type OBJECT/LIST/STRING/INTEGER/FLOAT got unknown value '%s'.", 
						 parser->location.line, parser->location.character, token.text);
			return NULL;
		}
		
	}
	return kvp;
}

JHL_STATIC b32
jhl_parse_object(Parser *parser, Json_Type *jt_object)
{
	b32 success = false;
	Token token; 
	
	// empty object.
	if (jhl_peek_next_token(parser, &token) && 
		token.token_type == TOKEN_RIGHT_CURLY)
	{
		jhl_get_next_token(parser, &token); // eat the token
		return false;
	}
	
	Json_Type *kvl_head = jhl_parse_key_value_pair(parser);
	if (!kvl_head) return false; 
	
	Json_Type *kvp = NULL;
    success = jhl_get_next_token(parser, &token);
	if (success && token.token_type == TOKEN_COMMA)
	{
		kvp = jhl_parse_key_value_pair(parser);
		kvl_head->object.next = kvp;
		
		success = jhl_get_next_token(parser, &token);
		while (kvp && success && token.token_type == TOKEN_COMMA)
		{
			kvp->object.next = jhl_parse_key_value_pair(parser);  
			kvp = kvp->object.next;
			
			success = jhl_get_next_token(parser, &token);
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
			
			JHL_LogError("error parsing(%d:%d): expected right curly bracked, got '%s'.", 
						 parser->location.line, parser->location.character, token.text)
		}
	}
	
	return false;
}

////////////////////////////////

JHL_STATIC b32 
jhl_array_add_element(Json_Type *arr, Json_Type elem)
{
	Assert(arr->type == TYPE_Array);
	Assert(elem.type > 0); // bad element.
	if (!arr->array.data)  return false;
	
	if (arr->array.capacity <= arr->array.index) 
	{
		// reallocate the array
		size_t new_size = (size_t)((double)arr->array.capacity * 1.5);
		Json_Type *new_array = (Json_Type *)jhl_mem_alloc(context, new_size * sizeof(Json_Type));
		memcpy(new_array, arr->array.data, arr->array.index * sizeof(Json_Type));
		arr->array.capacity = new_size;
		arr->array.data = new_array;
	}
	
	arr->array.data[arr->array.index++] = elem;
	return true;
}

////////////////////////////////

JHL_STATIC b32
jhl_parse_array(Parser *parser, Json_Type *jt_array)
{
	Token token; 
    b32 success = false;
	
	// handle empty list.
	if (jhl_peek_next_token(parser, &token) && token.token_type == TOKEN_RIGHT_BRACKET)
	{
		jhl_get_next_token(parser, &token); // eat that token.
		return false;
	}
	
	// create the array
	jt_array->type = TYPE_Array; 
	jt_array->array.capacity = 2; 
	jt_array->array.index    = 0;
	jt_array->array.data = (Json_Type *)jhl_mem_alloc(context, (sizeof(Json_Type) * jt_array->array.capacity));
	
	while (jhl_get_next_token(parser, &token)) 
	{
		
		if (token.token_type == TOKEN_LEFT_BRACKET)    // array 
		{
			Json_Type temp_arr = {0};
			if (!jhl_parse_array(parser, &temp_arr))        return false; // could not parse the array
			if (!jhl_array_add_element(jt_array, temp_arr)) return false; // out of memeory.
			
		}
		else if (token.token_type == TOKEN_LEFT_CURLY) // object
		{
			Json_Type temp_object = {0};
			if (!jhl_parse_object(parser, &temp_object))       return false; // could not parse the object
			if (!jhl_array_add_element(jt_array, temp_object)) return false; // out of memory.
			
		}
		else if (token.token_type == TOKEN_STRING)     // string
		{
			Json_Type jt_str = {0};
			jt_str.type = TYPE_String; 
			jt_str.string.data = token.text; 
			jt_str.string.size = token.text_size; 
			if (!jhl_array_add_element(jt_array, jt_str)) return false; // out of memory.
			
		}
		else if (token.token_type == TOKEN_INTEGER)    // integer 
		{
			Json_Type jt_int = {0};
			jt_int.type = TYPE_Integer; 
			
			// create the string slice that stores the number info
			jhl_string s = {0};
			s.size = token.text_size;
			s.data = token.text;
			if (!jhl_string_to_integer(s, &jt_int.integer)) 
			{
				JHL_LogError("error(%d:%d): internal error, could not convert a integer token to a integer", 
							 parser->location.line, parser->location.character);
				return false; // unable to convert to integer
			}
			
			if (!jhl_array_add_element(jt_array, jt_int)) return false; // out of memory
			
		}
		else if (token.token_type == TOKEN_FLOAT)      // float
		{
			Json_Type jt_float = {0}; 
			jt_float.type = TYPE_Float; 
			
			// create the string slice that stores the number info
			jhl_string s = {0};
			s.size = token.text_size;
			s.data = token.text;
			if (!jhl_string_to_float(s, &jt_float.floating_point)) 
			{
				// unsuccessful
				JHL_LogError("error(%d:%d): JHL_STATIC error, could not convert a integer token to a integer", 
							 parser->location.line, parser->location.character);
				return false;
			}
			
			if (!jhl_array_add_element(jt_array, jt_float)) return false; // out of memory
			
		}
		else if (token.token_type == TOKEN_RIGHT_BRACKET) 
		{
			break;
		}
		else // this is an incorrect type
		{
			JHL_LogError("error(%d:%d): unknown type found %d.",
						 parser->location.line, parser->location.character, (int)token.token_type);  
			return false;
		}
		
		if (jhl_peek_next_token(parser, &token) && token.token_type == TOKEN_COMMA) 
		{
			jhl_get_next_token(parser, &token); // go over the token
		}
		else if (!(token.token_type & (TOKEN_RIGHT_BRACKET | TOKEN_RIGHT_CURLY | 
									   TOKEN_FLOAT | TOKEN_INTEGER | TOKEN_STRING)))
		{
			
			JHL_LogError("error(%d:%d): missing comma when dividing between multiple types.", 
						 parser->location.line, parser->location.character); 
			return false;
		}
		
	}
	
	return true; 
}

JHL_STATIC Json_Type *
jhl_parse_json(char *input_buffer)
{
	// setup
	init_token_to_string_map();
	jhl_init_global_context(4 * 1024);
	
	Parser parser = {0};
	parser.text   = input_buffer; 
	parser.location.line = 1; // Most code editors begin at line 1. 
	
	Json_Type *head = (Json_Type *)jhl_mem_alloc(context, (sizeof(Json_Type)));
	
	// look and see if it begins with a curly brace or a bracket.
	Token token = {0}; 
	b32 success = jhl_get_next_token(&parser, &token);
	if (token.token_type == TOKEN_LEFT_CURLY)
	{
		if (!jhl_parse_object(&parser, head)) return NULL;
	}
	else if (token.token_type == TOKEN_LEFT_BRACKET)
	{
		if (!jhl_parse_array(&parser, head)) return NULL;
	}
	else 
	{
		JHL_LogError("error parsing(%d:%d): did not find object/list type at the beginning", 
					 parser.location.line, parser.location.character);
	}
	
	return head;
}

#endif //JSON_HL_H
