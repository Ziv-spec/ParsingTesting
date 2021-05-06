
#define internal static
#include "language.h" // Ignore for now, I will make it a part of the hl_json librarly soon.
#include "json_hl.h"  // The header library.

// This is a example that will grow in size soon. 
// The parser will soon be a semi finished state, 
// in which I would be able to really make a good 
// example usecase. 





// some really quick witted debug printing for the Json_Type. 
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
debug_print_value(Json_Type *value, s32 padding_level)
{
	
	if (value->type == TYPE_Object)
	{
		Json_Type *json_object = (Json_Type *)value;
		if (json_object)  debug_print_object(json_object->object.head, padding_level);
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
			// TODO(ziv): implement this !!!! please !!! 
		}
		else if (value->type == TYPE_Array)
		{
			//debug_print_array();
		}
		
	}
}

internal void
debug_print_key_value_pair(Key_Value_Node *kv, s32 padding_level)
{
	debug_print_padding(padding_level); Log("%c%s%c",'"', kv->key, '"'); 
	Log(": "); debug_print_value(kv->
								 padding_level); 
}


internal void
debug_print_object(Json_Type *jt_object, s32 padding_level)
{
	Log("{\n"); 
	Key_Value_List *obj = &jt_object->object;
	if (obj)
	{
		for (; obj->next; obj = obj->next)
		{
			debug_print_key_value_pair(obj, padding_level + 1);
			Log(", \n");
		}
		debug_print_key_value_pair(obj, padding_level + 1); Log("\n");
		debug_print_padding(padding_level); 
	}
	Log("}");
}

internal void
debug_print_array(Json_Type_Array *arr, s32 padding_level)
{
	Log("[\n"); 
	if (arr)
	{
		debug_print_padding(padding_level);
		for (int i = 0; i < arr->index; i++)
		{
			Json_Type jt = arr->data[i];
			if (jt.type == TYPE_String)
			{
				Log("%c%s%c", '"',(char *)slice_to_string(jt.string), '"'); 
			}
			else if (jt.type == TYPE_Integer)
			{
				Log("%zu", jt.integer);
			}
			else if (jt.type == TYPE_Float)
			{
				Log("%lf", jt.floating_point);
			}
			else if (jt.type == TYPE_Object)
			{
				// TODO(ziv): implement this !!!! please!!! 
			}
		}
		
	}
	Log("]\n"); 
	
}

internal void 
debug_print_json_from_json_node(Json_Type *json_type)
{
	if (json_type)
	{
		if (json_type->type == TYPE_Object)
		{
			debug_print_object(json_type, 0);
		}
		else if (json_type->type == TYPE_Array)
		{
			debug_print_array((Json_Type_Array *)&json_type->array, 0);
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

// TODO(ziv): add debug print list.



int main(int argc, char **argv)
{
    if (argc == 1) // TODO(ziv): Change this to 2. 
    {
        char *file_name = argv[1]; 
		
        file_name = "C:\\dev\\json\\json_example.json";  // TODO(ziv): Remove this. 
        file_name = "C:\\Users\\natal\\OneDrive\\Desktop\\super_large_json_file.txt";
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
                
                //fprintf(stdout, "Json data:\n%s\n\n", buffer); // prints out the file contents.
                
                fclose(file);
                
				Json_Type *json_tree = parse_json(buffer);
                
				// print the structure (visual representation)
				debug_print_json_from_json_node(json_tree);
				
				// This node is the AST tree head node. 
				//debug_print_ast_tree(ast_tree);
				
				free(buffer); // although, it should get freed at the end of the application.
				// so... I don't have to do this.
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


