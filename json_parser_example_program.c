#include <stdio.h>
#include <stdlib.h>

#define internal static
#include "language.h" // Ignore for now, I will make it a part of the hl_json librarly soon.
#include "json_hl.h"  // The header library.

// This is a example that will grow in size soon. 
// The parser will soon be a semi finished state, 
// in which I would be able to really make a good 
// example usecase. 

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
                buffer[file_size] = '\0';
                
                fprintf(stdout, "Json data:\n%s\n\n", buffer);
                
                fclose(file);
                
                // 
                // Begin the parsing  
                //
                
                Ast_Node *ast_tree = parse_json(buffer); 
                debug_print_json_from_ast_node(ast_tree);
				// This node is the AST tree head node. 
				//debug_print_ast_tree(ast_tree); 
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

