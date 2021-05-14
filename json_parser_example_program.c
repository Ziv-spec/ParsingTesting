#define internal static
#include "language.h" // Ignore for now, I will make it a part of the hl_json librarly soon.
#include "json_hl.h"  // The header library.

// some really quick witted debug printing for the Json_Type. 

internal void 
debug_print_padding(s32 padding_level)
{
	for (int padding_counter = 0; padding_counter < padding_level; padding_counter++)
	{
		Log("  ");
	}
}

internal void debug_print_object(Json_Type *object, s32 padding_level);
internal void debug_print_array(Json_Type *arr, s32 padding_level);

internal void
debug_print_value(Json_Type *value, s32 padding_level)
{
	
	if (value->type == TYPE_Object)
	{
		if (value)  debug_print_object(value, padding_level);
		else Log("{}");
	}
	else 
	{
		if (value->type == TYPE_String)
		{
			Log("%c%s%c", '"',(char *)slice_to_string(value->string), '"'); 
		}
		else if (value->type == TYPE_Integer)
		{
			Log("%zu", value->integer);
		}
		else if (value->type == TYPE_Float)
		{
			Log("%lf", value->floating_point);
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
				Log("true");
			}
			else
			{
				Log("false");
			}
		}
	}
}

internal void
debug_print_key_value_pair(string_slice key, Json_Type *value, s32 padding_level)
{
	// print the key
	debug_print_padding(padding_level); Log("%c%s%c",'"', key.data, '"');  
	
	// print the value
	Log(": "); debug_print_value(value, padding_level); 
}


internal void
debug_print_object(Json_Type *object, s32 padding_level)
{
	Assert(object->type == TYPE_Object);
	if (!object) return;
	
	Log("{\n"); 
	
	for (; object->object.next; object = object->object.next)
	{
		debug_print_key_value_pair(object->object.key,
								   object->object.value, 
								   padding_level + 1); Log(", \n");
	}
	debug_print_key_value_pair(object->object.key, 
							   object->object.value, 
							   padding_level + 1); Log("\n");
	
	debug_print_padding(padding_level); Log("}");
}

internal void
debug_print_array(Json_Type *arr, s32 padding_level)
{
	Assert(arr->type == TYPE_Array);
	
	Log("[\n"); 
	if (arr)
	{
		debug_print_padding(padding_level + 1); 
		
		int i = 0;
		for (; i < arr->array.index-1; i++)
		{
			Simple_Types type = arr->array.data[i].type;
			
			if (type == TYPE_String)
			{
				Log("%c%s%c", '"',(char *)slice_to_string(arr->array.data[i].string), '"'); 
				Log(",\n"); debug_print_padding(padding_level+1);
			}
			else if (type == TYPE_Integer)
			{
				Log("%zu", arr->array.data[i].integer);
				Log(",\n"); debug_print_padding(padding_level+1);
			}
			else if (type == TYPE_Float)
			{
				Log("%lf", arr->array.data[i].floating_point);
				Log(",\n"); debug_print_padding(padding_level+1);
			}
			else if (type == TYPE_Object)
			{
				debug_print_object(arr->array.data+i, padding_level + 1);
				Log(","); Log("\n"); debug_print_padding(padding_level + 1); 
			}
			else if (type == TYPE_Array)
			{
				debug_print_array(arr->array.data+i, padding_level + 1);
				Log(","); Log("\n"); debug_print_padding(padding_level + 1); 
			}
		}
		
		// last element without the ',' 
		Simple_Types type = arr->array.data[i].type;
		if (type == TYPE_String)
		{
			Log("%c%s%c", '"',(char *)slice_to_string(arr->array.data[i].string), '"'); 
		}
		else if (type == TYPE_Integer)
		{
			Log("%zu", arr->array.data[i].integer);
		}
		else if (type == TYPE_Float)
		{
			Log("%lf", arr->array.data[i].floating_point);
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
	
	Log("\n"); debug_print_padding(padding_level); Log("]"); 
}

internal void 
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
		Log("\n");
	}
}

internal void
debug_print_token(Token token)
{
    
    char *token_name = token_name_map[token.token_type];
    
    char padding[255];
    s32 longest_size = string_size("TOKEN_RIGHT_BRACKET") - 1;
    s32 token_string_size = string_size(token_name);
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
                
				Json_Type *json_tree = parse_json(buffer);
				// print the structure (visual representation)
				debug_print_json_tree(json_tree);
				
                //context.free(); // free the memory that holds the json tree.
				free(buffer); // although, it should get freed at the end of the application.
				// so... there is no need to do this in here. 
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