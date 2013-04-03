/*
    solid2obj
    Copyright (C) 2013 melchips (Francois Truphemus : francois (at) truphemus (dot) com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PROGRAM_NAME "solid2obj"
#define PROGRAM_VERSION "0.2a"
#define PROGRAM_DESCRIPTION "Wolfire's Black Shades solid file converter to obj file"

/* vertex 3d coordinates structure */
typedef struct _XYZ
{
    float x, y, z;
} XYZ_t;

/* textured triangle (colored triangle) structure */
typedef struct _textured_triangle
{
    short vertex[3];
    float r, g, b;
} textured_triangle_t;

/* solid mesh structure */
typedef struct _solid_mesh
{
    char filename[1024];
    short vertex_count;
    short triangle_count;
    XYZ_t *vertices;
    textured_triangle_t *triangles;
} solid_mesh_t;

/* basic generic list structure */
typedef struct _list
{
    void *data;
    struct _list *next;
} list_t;

/* material structure */
typedef struct _material
{
    char name[1024];
    int id;
    float r, g, b;
} material_t;

/* allocate a new list and returns its handle */
list_t * list_create(void *data)
{
    list_t *new_list;

    new_list = (list_t *) malloc(sizeof(list_t));

    if (new_list == NULL)
    {
        printf("Error while allocating list_t in function list_create !\n");
        return NULL;
    }

    new_list->next = NULL;
    new_list->data = data;

    return new_list;
}

/* remove the first element of the list and returns the new list handle */
list_t * list_pop_front(list_t *root)
{
    list_t *node_to_free = NULL;

    if (root == NULL)
        return NULL;

    node_to_free = root;
    root = root->next;
    free(node_to_free);
    return root;
}

/* return the data of the first element of the list */
void * list_get_front_data(list_t *root)
{
    if (root == NULL)
        return NULL;

    return root->data;
}

/* insert a new data in the list */
list_t * list_insert_front(list_t *root, void *data)
{
    list_t *new_root = NULL;

    new_root = list_create(data);

    new_root->next = root;

    return new_root;
}

/* get size of the list */
size_t list_size(list_t *root)
{
    size_t size = 0;
    list_t *iterator = NULL;

    iterator = root;
    while (iterator != NULL)
    {
        size++;
        iterator = iterator->next;
    }

    return size;
}

/* free memory used by a list, free data if specified */
list_t * list_free(list_t *root, int free_data)
{
    list_t *node_to_free = NULL;

    node_to_free = root;

    while (node_to_free != NULL)
    {
        root = node_to_free->next;
        if (free_data)
        {
            free(node_to_free->data);
        }
        free(node_to_free);
        node_to_free = root;
    }

    return root;
}

/* compare two materials (id is not compared) */
int compare_materials(const material_t *mat1, const material_t *mat2)
{
   return (mat1->r == mat2->r && mat1->g == mat2->g && mat1->b == mat2->b);
}

/* get the material matching the data supplied */
material_t * material_get(list_t *root, const material_t *material)
{
    list_t *iterator = NULL;

    /* go to the start of the list */
    iterator = root;

    while (iterator != NULL)
    {
        if ( compare_materials(material, iterator->data) )
            return iterator->data;

        iterator = iterator->next;
    }

    /* material not found */
    return NULL;
}

/* get the material matching the data supplied or insert a new one (list root passed as argument may be altered) */
material_t * material_get_or_insert_in_list(list_t **root, const material_t *material)
{
    material_t *new_material = NULL;
    material_t *found_material = NULL;

    found_material = material_get(*root, material);

    if (found_material != NULL)
        return found_material;

    /* material not found, insert a new one */
    new_material = (material_t *) malloc(sizeof(material_t));
    if (new_material == NULL)
    {
        printf("Error : can't allocate a new material_t in function material_get_or_insert_in_list !\n");
        return NULL;
    }
    *new_material = *material;

    *root = list_insert_front(*root, new_material);

    return new_material;
}

/* assign unique id and name to each material in the list */
void material_list_assign_unique_id_and_name(list_t *root)
{
    list_t *iterator = NULL;
    int current_unique_id = 0;
    char name_buffer[1024];

    iterator = root;

    while(iterator != NULL)
    {
        if (iterator->data != NULL)
        {
            ((material_t *) iterator->data)->id = current_unique_id;
            sprintf(name_buffer, "%s_%d", "material", current_unique_id);
            strncpy(((material_t *) iterator->data)->name, name_buffer, strlen(name_buffer));
            current_unique_id++;
        }
        iterator = iterator->next;
    }
}

/* read short (2 bytes) from file */
int read_short(FILE *file, int count, short *s)
{
    while(count--)
    {
        unsigned char buf[2];
        if (fread(&buf, 2, 1, file) != 1)
        {
            return 0;
        }
        *s = (short) ((buf[0] << 8) | buf[1]);
        s++;
    }
    return 1;
}

/* read int (4 bytes) from file */
int read_int(FILE *file, int count, int *s)
{
    while(count--)
    {
        unsigned char buf[4];
        if (fread(&buf, 4, 1, file) != 1)
        {
            return 0;
        }
        *s = (int) ((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | (buf[3]));
        s++;
    }
    return 1;
}

/* union between int and float (used here to convert int to float) */
union intfloat
{
    int i;
    float f;
} intfloat;

/* read float (here 4 bytes specifically to the solid file format) from file */
int read_float(FILE *file, int count, float *f)
{
    union intfloat infl;
    infl.f = 0;
    while(count--)
    {
       read_int(file, 1, &(infl.i));
       *f = infl.f;
       f++;
    }
    return 1;
}

/* read XYZ from file */
int read_XYZ(FILE *file, int count, XYZ_t *xyz)
{
    while(count--)
    {
        read_float(file, 1, &(xyz->x));
        read_float(file, 1, &(xyz->y));
        read_float(file, 1, &(xyz->z));
        xyz++;
    }
    return 1;
}

/* read textured_triangle from file */
int read_textured_triangle(FILE *file, int count, textured_triangle_t *textured_triangle)
{
    while(count--)
    {
        short pad;
        read_short(file, 3, textured_triangle->vertex);
        read_short(file, 1, &pad);
        read_float(file, 1, &(textured_triangle->r));
        read_float(file, 1, &(textured_triangle->g));
        read_float(file, 1, &(textured_triangle->b));
        textured_triangle++;
    }
    return 1;
}

solid_mesh_t *solid_mesh_create(char *filename, short vertex_count, short triangle_count)
{
    solid_mesh_t *solid_mesh = NULL;

    /* allocate memory for the solid mesh structure */
    solid_mesh = (solid_mesh_t *) malloc(sizeof(solid_mesh_t));
    if (solid_mesh == NULL)
    {
        printf("Error : can't allocate solid mesh in function create_solid_mesh !\n");
        return NULL;
    }

    /* set filename */
    strncpy(solid_mesh->filename, filename, 1024);

    /* set vertex and triangle count */
    solid_mesh->vertex_count = vertex_count;
    solid_mesh->triangle_count = triangle_count;

    /* allocate memory for the vertices of the solid mesh */
    solid_mesh->vertices = (XYZ_t *) malloc(sizeof(XYZ_t) * vertex_count);
    if (solid_mesh->vertices == NULL)
    {
        printf("Error : can't allocate solid mesh vertices in function create_solid_mesh !\n");
        free(solid_mesh);
        return NULL;
    }

    /* allocate memory for the triangles of the solid mesh */
    solid_mesh->triangles = (textured_triangle_t *) malloc(sizeof(textured_triangle_t) * triangle_count);
    if (solid_mesh->triangles == NULL)
    {
        printf("Error : can't allocate solid mesh triangles in function create_solid_mesh !\n");
        free(solid_mesh->vertices);
        free(solid_mesh);
        return NULL;
    }

    /* return the newly created solid mesh */
    return solid_mesh;
}

void solid_mesh_free(solid_mesh_t *solid_mesh)
{
    if (solid_mesh == NULL)
        return;

    if (solid_mesh->vertices != NULL)
        free(solid_mesh->vertices);

    if (solid_mesh->triangles != NULL)
        free(solid_mesh->triangles);

    free(solid_mesh);
}

void solid_mesh_convert_to_obj(solid_mesh_t *solid_mesh, char *output_file_path, char *output_material_file_path)
{
    FILE *output_file = NULL;
    FILE *output_material_file = NULL;
    int vertex_index = 0;
    int triangle_index = 0;
    list_t *material_list = NULL;
    material_t *current_material = NULL;
    material_t *found_material = NULL; 
    int previous_material_id = -1;
    list_t *iterator = NULL;

    if (solid_mesh == NULL)
    {
        printf("Error : solid mesh is NULL in function solid_mesh_convert_to_obj !\n");
        return;
    }

    /* allocate memory for the material used for comparison */
    current_material = (material_t *) malloc(sizeof(material_t));

    /* compute all materials (colors only) */
    for(triangle_index=0;triangle_index<solid_mesh->triangle_count;triangle_index++)
    {
        current_material->r =  solid_mesh->triangles[triangle_index].r;
        current_material->g =  solid_mesh->triangles[triangle_index].g;
        current_material->b =  solid_mesh->triangles[triangle_index].b;
        found_material = material_get_or_insert_in_list(&material_list, current_material);
    }
    material_list_assign_unique_id_and_name(material_list);
        printf("%lu material(s) declared\n", list_size(material_list));
  
    /* OBJ file */

    /* open obj output file for writing */
    output_file = fopen(output_file_path, "w");
    if (output_file == NULL)
    {
        printf("Error : can't open '%s' for writing !\n", output_file_path);
        return;
    }
    
    /* header */
    fprintf(output_file, "# exported from Blackshade's solid mesh file '%s'\n", solid_mesh->filename);

    /* material file */
    fprintf(output_file, "mtllib %s\n", output_material_file_path); 

    /* export vertices */
    for(vertex_index=0;vertex_index<solid_mesh->vertex_count;vertex_index++)
    {
        fprintf(output_file, "v %f %f %f 1.0\n",
                solid_mesh->vertices[vertex_index].x,
                solid_mesh->vertices[vertex_index].y,
                solid_mesh->vertices[vertex_index].z);
    }

    /* export triangles */
    for(triangle_index=0;triangle_index<solid_mesh->triangle_count;triangle_index++)
    {
        found_material = NULL;

        current_material->r =  solid_mesh->triangles[triangle_index].r;
        current_material->g =  solid_mesh->triangles[triangle_index].g;
        current_material->b =  solid_mesh->triangles[triangle_index].b;

        /* get matching material */
        found_material = material_get(material_list, current_material);

        if (found_material != NULL && found_material->id != previous_material_id)
        {
            fprintf(output_file, "usemtl %s\n", found_material->name);
            previous_material_id = found_material->id;
        }
        fprintf(output_file, "f %hu %hu %hu\n",
                solid_mesh->triangles[triangle_index].vertex[0] + 1,
                solid_mesh->triangles[triangle_index].vertex[1] + 1,
                solid_mesh->triangles[triangle_index].vertex[2] + 1
              );
    }

    fclose(output_file);

    /* MTL file */
    output_material_file = fopen(output_material_file_path, "w");
    if (output_material_file == NULL)
    {
        printf("Error : can't open '%s' for writing !\n", output_material_file_path);
        return;
    }

    /* header */
    fprintf(output_material_file, "# material file for Blackshade's solid mesh file '%s' converted to obj '%s'\n", solid_mesh->filename, output_file_path);

    /* export materials */
    iterator = material_list;

    while (iterator != NULL)
    {
        if (iterator->data != NULL)
        {
            fprintf(output_material_file, "\nnewmtl %s\n", ((material_t *)iterator->data)->name);
            /* ambient color */
            fprintf(output_material_file, "Ka 1.0 1.0 1.0\n");
            /* diffuse color */
            fprintf(output_material_file, "Kd %f %f %f\n",
                    ((material_t *)iterator->data)->r,
                    ((material_t *)iterator->data)->g,
                    ((material_t *)iterator->data)->b
                   );

            /* specular color */
            fprintf(output_material_file, "Ks 0.0 0.0 0.0\n");
            fprintf(output_material_file, "Ns 0.0\n");
        }
        iterator = iterator->next;
    }

    fclose(output_material_file);

    /* free data */
    free(current_material);
    list_free(material_list, 1);
}

/* print usage of the command */
void usage(char *command_name)
{
    printf( PROGRAM_NAME " v." PROGRAM_VERSION " :\n"
            "\t" PROGRAM_DESCRIPTION "\n\n"
            "usage:\n"
            "%s <input_solid_file> <output_obj_file> <output_mtl_file>\n"
            "\tinput_solid_file \t:\ta valid solid mesh file\n"
            "\toutput_obj_file \t:\tname of the output obj file to create\n"
            "\toutput_mtl_file \t:\tname of the output material file to create\n"
            "\n"
            "\t! WARNING output files WILL be OVERWRITTEN !\n"
            , command_name);
}


int main(int argc, char *argv[])
{
    /* path to the input and output mesh files */
    char solid_file_path[1024];
    char obj_file_path[1024];
    char obj_material_file_path[1024];

    /* solid mesh file handle */
    FILE *solid_file = NULL;

    /* solid mesh */
    solid_mesh_t *solid_mesh = NULL;

    /* number of vertices to read from the solid file */
    short vertex_count = 0;
    /* number of triangles to read from the solid file */
    short triangle_count = 0;

    /* initialize paths with '\0' */
    memset(solid_file_path, '\0', 1024);
    memset(obj_file_path, '\0', 1024);
    memset(obj_material_file_path, '\0', 1024);

    /* if files are specified at command line */
    if (argc > 3)
    {
        /* copy files names into corresponding arrays */
        strncpy(solid_file_path, argv[1], 1024);
        strncpy(obj_file_path, argv[2], 1024);
        strncpy(obj_material_file_path, argv[3], 1024);

        printf("loading '%s'...\n", solid_file_path);

        /* opening the file in binary mode */
        solid_file = fopen(solid_file_path, "rb");
        if (solid_file == NULL)
        {
            printf("can't load file '%s' !\n", solid_file_path);
            exit(2);
        }
        
        /* read vertices count in solid file */
        read_short(solid_file, 1, &vertex_count);
        printf("%d vertices to read\n", vertex_count);

        /* read triangles count in solid file */
        read_short(solid_file, 1, &triangle_count);
        printf("%d triangles to read\n", triangle_count);

        /* allocate memory for the solid mesh */
        solid_mesh = solid_mesh_create(solid_file_path, vertex_count, triangle_count);

        /* fill the solid mesh from file */
        read_XYZ(solid_file, vertex_count, solid_mesh->vertices);
        read_textured_triangle(solid_file, triangle_count, solid_mesh->triangles);

        fclose(solid_file);

        /* dump data */
        for(vertex_count=0;vertex_count<solid_mesh->vertex_count;vertex_count++)
        {
            printf("XYZ[%f, %f, %f]\n", solid_mesh->vertices[vertex_count].x, solid_mesh->vertices[vertex_count].y, solid_mesh->vertices[vertex_count].z);
        }

        for(triangle_count=0;triangle_count<solid_mesh->triangle_count;triangle_count++)
        {
            printf("tri[%hu, %hu, %hu rgb(%G, %G, %G)]\n", solid_mesh->triangles[triangle_count].vertex[0],
                    solid_mesh->triangles[triangle_count].vertex[1],
                    solid_mesh->triangles[triangle_count].vertex[2],
                    solid_mesh->triangles[triangle_count].r,
                    solid_mesh->triangles[triangle_count].g,
                    solid_mesh->triangles[triangle_count].b
                  );
        }

        /* create obj file */
        printf("creating obj file...\n");
        solid_mesh_convert_to_obj(solid_mesh, obj_file_path, obj_material_file_path);
        printf("...done !\n");

        /* free data */
        solid_mesh_free(solid_mesh);
    } else {
        usage(argv[0]);
        exit(1);
    }
    return 0;
}

