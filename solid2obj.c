/*
    solid2obj
    Copyright (C) 2013 melchips (Francois Truphemus : francois (at) truphemus (dot) fr)

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
#define PROGRAM_VERSION "0.3.1a"
#define PROGRAM_DESCRIPTION "Wolfire's Black Shades solid file converter from and to obj file"

#define BLACK_SHADES_MAX_FACES 400
#define BLACK_SHADES_MAX_VERTICES BLACK_SHADES_MAX_FACES*3

/* solid file vertex 3d coordinates structure */
typedef struct _solid_XYZ
{
    float x, y, z;
} solid_XYZ_t;

/* solid file textured triangle (colored triangle) structure */
typedef struct _solid_textured_triangle
{
    short vertex[3];
    float r, g, b;
} solid_textured_triangle_t;

/* solid file mesh structure */
typedef struct _solid_mesh
{
    char filename[1024];
    short vertex_count;
    short triangle_count;
    solid_XYZ_t *vertices;
    solid_textured_triangle_t *triangles;
} solid_mesh_t;

/* solid file material structure */
typedef struct _solid_material
{
    char name[1024];
    int id;
    float r, g, b;
} solid_material_t;

/* vertex in obj file */
typedef struct _obj_vextex
{
    float x, y, z, w;
} obj_vertex_t;

/* face in obj file */
typedef struct _obj_face
{
    /* face can be a triangle or a quad */
    int is_quad;
    int vertex_index[4];
    int texture_index[4];
    int normal_index[4];
    char texture_name[1024];
} obj_face_t;

/* material in obj file */
typedef struct _obj_material
{
    char name[1024];
    float ambient_r, ambient_g, ambient_b;
    float diffuse_r, diffuse_g, diffuse_b;
    float specular_r, specular_g, specular_b;
    float specular_coefficient;
} obj_material_t;

/* obj file mesh structure */
typedef struct _obj_mesh
{
    char filename[1024];
    char material_filename[1024];
    int vertices_used;
    int vertices_allocated;
    obj_vertex_t *vertices;
    int faces_used;
    int faces_allocated;
    obj_face_t *faces;
    int materials_used;
    int materials_allocated;
    obj_material_t *materials;
} obj_mesh_t;

/* basic generic list structure */
typedef struct _list
{
    void *data;
    struct _list *next;
} list_t;

/* create a new vertex in a obj mesh and return its handle */
obj_vertex_t * obj_add_vertex(obj_mesh_t *obj_mesh)
{
    obj_vertex_t *new_buffer = NULL;
    obj_vertex_t *new_vertex = NULL;
    int vertex_array_increment = 1; /* allocate X vertex at a time */

    if (obj_mesh == NULL)
        return NULL;

    /* reserved memory is full or has not yet been created */
    if (obj_mesh->vertices_used == obj_mesh->vertices_allocated)
        {
            new_buffer = (obj_vertex_t *) realloc(obj_mesh->vertices, sizeof(obj_vertex_t) * (obj_mesh->vertices_allocated + vertex_array_increment));
            if (new_buffer != NULL)
                {
                    obj_mesh->vertices = new_buffer;
                    obj_mesh->vertices_allocated += vertex_array_increment;
                }
            else
                {
                    printf("Error : can't realloc vertices buffer in function obj_add_vertex !\n");
                    return NULL;
                }
        }

    /* if there is still unused vertices */
    if (obj_mesh->vertices_used < obj_mesh->vertices_allocated)
        {
            new_vertex = obj_mesh->vertices + obj_mesh->vertices_used;
            obj_mesh->vertices_used++;
            return new_vertex;
        }

    return NULL;
}

/* create a new face in a obj mesh and return its handle */
obj_face_t * obj_add_face(obj_mesh_t *obj_mesh)
{
    obj_face_t *new_buffer = NULL;
    obj_face_t *new_face = NULL;
    int face_array_increment = 1; /* allocate X face at a time */

    if (obj_mesh == NULL)
        return NULL;

    /* reserved memory is full or has not yet been created */
    if (obj_mesh->faces_used == obj_mesh->faces_allocated)
        {
            new_buffer = (obj_face_t *) realloc(obj_mesh->faces, sizeof(obj_face_t) * (obj_mesh->faces_allocated + face_array_increment));
            if (new_buffer != NULL)
                {
                    obj_mesh->faces = new_buffer;
                    obj_mesh->faces_allocated += face_array_increment;
                }
            else
                {
                    printf("Error : can't realloc faces buffer in function obj_add_face !\n");
                    return NULL;
                }
        }

    /* if there is still unused faces */
    if (obj_mesh->faces_used < obj_mesh->faces_allocated)
        {
            new_face = obj_mesh->faces + obj_mesh->faces_used;
            obj_mesh->faces_used++;
            return new_face;
        }

    return NULL;
}

/* create a new material in a obj mesh and return its handle */
obj_material_t * obj_add_material(obj_mesh_t *obj_mesh)
{
    obj_material_t *new_buffer = NULL;
    obj_material_t *new_material = NULL;
    int material_array_increment = 1; /* allocate X material at a time */

    if (obj_mesh == NULL)
        return NULL;

    /* reserved memory is full or has not yet been created */
    if (obj_mesh->materials_used == obj_mesh->materials_allocated)
        {
            new_buffer = (obj_material_t *) realloc(obj_mesh->materials, sizeof(obj_material_t) * (obj_mesh->materials_allocated + material_array_increment));
            if (new_buffer != NULL)
                {
                    obj_mesh->materials = new_buffer;
                    obj_mesh->materials_allocated += material_array_increment;
                }
            else
                {
                    printf("Error : can't realloc materials buffer in function obj_add_material !\n");
                    return NULL;
                }
        }

    /* if there is still unused materials */
    if (obj_mesh->materials_used < obj_mesh->materials_allocated)
        {
            new_material = obj_mesh->materials + obj_mesh->materials_used;
            obj_mesh->materials_used++;
            return new_material;
        }

    return NULL;
}

/* create a new obj mesh in memory */
obj_mesh_t * obj_mesh_create(char *filename)
{
    obj_mesh_t *obj_mesh = NULL;

    obj_mesh = (obj_mesh_t *) malloc(sizeof(obj_mesh_t));

    if (obj_mesh == NULL)
        {
            printf("Error : can't allocate obj_mesh_t in function obj_mesh_create !\n");
            return NULL;
        }

    /* init filename */
    strncpy(obj_mesh->filename, filename, 1024);

    /* init material filename (we need to parse the file to get the real name of the mat file) */
    memset(obj_mesh->material_filename,'\0', 1024);

    /* init vertices */
    obj_mesh->vertices = NULL;
    obj_mesh->vertices_used = 0;
    obj_mesh->vertices_allocated = 0;

    /* init faces */
    obj_mesh->faces = NULL;
    obj_mesh->faces_used = 0;
    obj_mesh->faces_allocated = 0;

    /* init materials */
    obj_mesh->materials = NULL;
    obj_mesh->materials_used = 0;
    obj_mesh->materials_allocated = 0;

    return obj_mesh;
}

/* free a obj mesh from memory */
void obj_mesh_free(obj_mesh_t *obj_mesh)
{
    if (obj_mesh == NULL)
        return;

    if (obj_mesh->vertices != NULL)
        free(obj_mesh->vertices);

    if (obj_mesh->faces != NULL)
        free(obj_mesh->faces);

    free(obj_mesh);
}

obj_face_t * obj_read_face(char *read_line, obj_mesh_t *obj_mesh)
{
    obj_face_t *obj_face;

    obj_face = obj_add_face(obj_mesh);

    if (obj_face != NULL)
        {
            obj_face->is_quad = 0;
            obj_face->vertex_index[0]=1;
            obj_face->vertex_index[1]=1;
            obj_face->vertex_index[2]=1;
            obj_face->vertex_index[3]=1;
            obj_face->texture_index[0]=1;
            obj_face->texture_index[1]=1;
            obj_face->texture_index[2]=1;
            obj_face->texture_index[3]=1;
            obj_face->normal_index[0]=1;
            obj_face->normal_index[1]=1;
            obj_face->normal_index[2]=1;
            obj_face->normal_index[3]=1;
            memset(obj_face->texture_name, '\0', 1024);

            /* QUAD with vertex / texture / normal */
            if (sscanf(read_line, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
                       &obj_face->vertex_index[0],
                       &obj_face->texture_index[0],
                       &obj_face->normal_index[0],
                       &obj_face->vertex_index[1],
                       &obj_face->texture_index[1],
                       &obj_face->normal_index[1],
                       &obj_face->vertex_index[2],
                       &obj_face->texture_index[2],
                       &obj_face->normal_index[2],
                       &obj_face->vertex_index[3],
                       &obj_face->texture_index[3],
                       &obj_face->normal_index[3]
                      ) == 12)
                {
                    obj_face->is_quad = 1;
                    return obj_face;
                }

            /* QUAD with vertex / texture */
            if (sscanf(read_line, "f %d/%d %d/%d %d/%d %d/%d",
                       &obj_face->vertex_index[0],
                       &obj_face->texture_index[0],
                       &obj_face->vertex_index[1],
                       &obj_face->texture_index[1],
                       &obj_face->vertex_index[2],
                       &obj_face->texture_index[2],
                       &obj_face->vertex_index[3],
                       &obj_face->texture_index[3]
                      ) == 8)
                {
                    obj_face->is_quad = 1;
                    return obj_face;
                }

            /* QUAD with vertex */
            if (sscanf(read_line, "f %d %d %d %d",
                       &obj_face->vertex_index[0],
                       &obj_face->vertex_index[1],
                       &obj_face->vertex_index[2],
                       &obj_face->vertex_index[3]
                      ) == 4)
                {
                    obj_face->is_quad = 1;
                    return obj_face;
                }

            /* TRI with vertex / texture / normal */
            if (sscanf(read_line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                       &obj_face->vertex_index[0],
                       &obj_face->texture_index[0],
                       &obj_face->normal_index[0],
                       &obj_face->vertex_index[1],
                       &obj_face->texture_index[1],
                       &obj_face->normal_index[1],
                       &obj_face->vertex_index[2],
                       &obj_face->texture_index[2],
                       &obj_face->normal_index[2]
                      ) == 9)
                {
                    return obj_face;
                }

            /* TRI with vertex / texture */
            if (sscanf(read_line, "f %d/%d %d/%d %d/%d",
                       &obj_face->vertex_index[0],
                       &obj_face->texture_index[0],
                       &obj_face->vertex_index[1],
                       &obj_face->texture_index[1],
                       &obj_face->vertex_index[2],
                       &obj_face->texture_index[2]
                      ) == 6)
                {
                    return obj_face;
                }

            /* TRI with vertex */
            if (sscanf(read_line, "f %d %d %d",
                       &obj_face->vertex_index[0],
                       &obj_face->vertex_index[1],
                       &obj_face->vertex_index[2]
                      ) == 3)
                {
                    return obj_face;
                }


            return obj_face;

            printf("Unknown face format\n");
        }
    return NULL;
}

obj_material_t * obj_get_material_by_name(obj_mesh_t *obj_mesh, char *name)
{
    int material_index = 0;

    if (obj_mesh == NULL)
        {
            return NULL;
        }

    for(material_index=0; material_index < obj_mesh->materials_used; material_index++)
        {
            if ( strcmp(obj_mesh->materials[material_index].name, name) == 0)
                {
                    return &obj_mesh->materials[material_index];
                }
        }

    return NULL;
}

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

/* compare two materials colors (id is not compared) */
int compare_materials_colors(const solid_material_t *mat1, const solid_material_t *mat2)
{
    return (mat1->r == mat2->r && mat1->g == mat2->g && mat1->b == mat2->b);
}

/* get the material matching the data supplied */
solid_material_t * solid_material_get(list_t *root, const solid_material_t *material)
{
    list_t *iterator = NULL;

    /* go to the start of the list */
    iterator = root;

    while (iterator != NULL)
        {
            if ( compare_materials_colors(material, iterator->data) )
                return iterator->data;

            iterator = iterator->next;
        }

    /* material not found */
    return NULL;
}

/* get the material matching the data supplied or insert a new one (list root passed as argument may be altered) */
solid_material_t * solid_material_get_or_insert_in_list(list_t **root, const solid_material_t *material)
{
    solid_material_t *new_material = NULL;
    solid_material_t *found_material = NULL;

    found_material = solid_material_get(*root, material);

    if (found_material != NULL)
        return found_material;

    /* material not found, insert a new one */
    new_material = (solid_material_t *) malloc(sizeof(solid_material_t));
    if (new_material == NULL)
        {
            printf("Error : can't allocate a new solid_material_t in function solid_material_get_or_insert_in_list !\n");
            return NULL;
        }
    *new_material = *material;

    *root = list_insert_front(*root, new_material);

    return new_material;
}

/* assign unique id and name to each material in the list */
void solid_material_list_assign_unique_id_and_name(list_t *root)
{
    list_t *iterator = NULL;
    int current_unique_id = 0;
    char name_buffer[1024];

    iterator = root;

    while(iterator != NULL)
        {
            if (iterator->data != NULL)
                {
                    ((solid_material_t *) iterator->data)->id = current_unique_id;
                    sprintf(name_buffer, "%s_%d", "material", current_unique_id);
                    strncpy(((solid_material_t *) iterator->data)->name, name_buffer, strlen(name_buffer));
                    current_unique_id++;
                }
            iterator = iterator->next;
        }
}

/* read short (2 bytes) from file */
int solid_read_short(FILE *file, int count, short *s)
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

/* write short (2 bytes) to file */
int solid_write_short(FILE *file, int count, const short *s)
{
    while(count--)
        {
            fputc( (*s >> 8) & 0xff, file);
            fputc( (*s >> 0) & 0xff, file);
            s++;
        }
    return 1;
}

/* read int (4 bytes) from file */
int solid_read_int(FILE *file, int count, int *s)
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

/* write int (4 bytes) to file */
int solid_write_int(FILE *file, int count, const int *s)
{
    while(count--)
        {
            fputc( (*s >> 24) & 0xff, file);
            fputc( (*s >> 16) & 0xff, file);
            fputc( (*s >> 8) & 0xff, file);
            fputc( (*s >> 0) & 0xff, file);
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
int solid_read_float(FILE *file, int count, float *f)
{
    union intfloat infl;
    infl.f = 0;
    while(count--)
        {
            solid_read_int(file, 1, &(infl.i));
            *f = infl.f;
            f++;
        }
    return 1;
}

/* write float (4 bytes) to file */
int solid_write_float(FILE *file, int count, const float *f)
{
    union intfloat infl;
    while(count--)
        {
            infl.f = *f;
            solid_write_int(file, 1, &infl.i);
            f++;
        }
    return 1;
}

/* read XYZ from file */
int solid_read_XYZ(FILE *file, int count, solid_XYZ_t *xyz)
{
    while(count--)
        {
            solid_read_float(file, 1, &(xyz->x));
            solid_read_float(file, 1, &(xyz->y));
            solid_read_float(file, 1, &(xyz->z));
            xyz++;
        }
    return 1;
}

/* read solid_textured_triangle from file */
int solid_read_textured_triangle(FILE *file, int count, solid_textured_triangle_t *solid_textured_triangle)
{
    while(count--)
        {
            short pad;
            solid_read_short(file, 3, solid_textured_triangle->vertex);
            solid_read_short(file, 1, &pad);
            solid_read_float(file, 1, &(solid_textured_triangle->r));
            solid_read_float(file, 1, &(solid_textured_triangle->g));
            solid_read_float(file, 1, &(solid_textured_triangle->b));
            solid_textured_triangle++;
        }
    return 1;
}

/* create a new solid mesh in memory */
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
    solid_mesh->vertices = (solid_XYZ_t *) malloc(sizeof(solid_XYZ_t) * vertex_count);
    if (solid_mesh->vertices == NULL)
        {
            printf("Error : can't allocate solid mesh vertices in function create_solid_mesh !\n");
            free(solid_mesh);
            return NULL;
        }

    /* allocate memory for the triangles of the solid mesh */
    solid_mesh->triangles = (solid_textured_triangle_t *) malloc(sizeof(solid_textured_triangle_t) * triangle_count);
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

/* free a solid mesh from memory */
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

/* convert a solid mesh to an obj one */
void solid_mesh_convert_to_obj(solid_mesh_t *solid_mesh, char *output_file_path, char *output_material_file_path)
{
    FILE *output_file = NULL;
    FILE *output_material_file = NULL;
    int vertex_index = 0;
    int triangle_index = 0;
    list_t *material_list = NULL;
    solid_material_t *current_material = NULL;
    solid_material_t *found_material = NULL;
    int previous_material_id = -1;
    list_t *iterator = NULL;

    if (solid_mesh == NULL)
        {
            printf("Error : solid mesh is NULL in function solid_mesh_convert_to_obj !\n");
            return;
        }

    /* allocate memory for the material used for comparison */
    current_material = (solid_material_t *) malloc(sizeof(solid_material_t));

    /* compute all materials (colors only) */
    for(triangle_index=0; triangle_index<solid_mesh->triangle_count; triangle_index++)
        {
            current_material->r =  solid_mesh->triangles[triangle_index].r;
            current_material->g =  solid_mesh->triangles[triangle_index].g;
            current_material->b =  solid_mesh->triangles[triangle_index].b;
            found_material = solid_material_get_or_insert_in_list(&material_list, current_material);
        }
    solid_material_list_assign_unique_id_and_name(material_list);
    printf("%lu material(s) declared\n", (unsigned long) list_size(material_list));

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
    for(vertex_index=0; vertex_index<solid_mesh->vertex_count; vertex_index++)
        {
            fprintf(output_file, "v %f %f %f 1.0\n",
                    solid_mesh->vertices[vertex_index].x,
                    -1 * solid_mesh->vertices[vertex_index].z, /* Up vector must be swapped for blender (blender uses Z as the up vector) */
                    solid_mesh->vertices[vertex_index].y
                   );
        }

    /* export triangles */
    for(triangle_index=0; triangle_index<solid_mesh->triangle_count; triangle_index++)
        {
            found_material = NULL;

            current_material->r =  solid_mesh->triangles[triangle_index].r;
            current_material->g =  solid_mesh->triangles[triangle_index].g;
            current_material->b =  solid_mesh->triangles[triangle_index].b;

            /* get matching material */
            found_material = solid_material_get(material_list, current_material);

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
                    fprintf(output_material_file, "\nnewmtl %s\n", ((solid_material_t *)iterator->data)->name);
                    /* ambient color */
                    fprintf(output_material_file, "Ka 1.0 1.0 1.0\n");
                    /* diffuse color */
                    fprintf(output_material_file, "Kd %f %f %f\n",
                            ((solid_material_t *)iterator->data)->r,
                            ((solid_material_t *)iterator->data)->g,
                            ((solid_material_t *)iterator->data)->b
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

/* convert an obj mesh to a solid one */
void obj_mesh_convert_to_solid(obj_mesh_t *obj_mesh, char *solid_file_path)
{
    FILE *output_file = NULL;
    obj_material_t *current_material = NULL;
    int vertex_index = 0;
    int face_index = 0;
    short vertices_count = 0;
    short faces_count = 0;
    short pad = 0;
    short vertex_index_short = 0;
    float color = 0.0f;
    float vertex_coordinate = 0.0f;

    if (obj_mesh == NULL)
        {
            printf("Error : obj mesh is NULL in function obj_mesh_convert_to_solid !\n");
            return;
        }

    /* Check sizes */
    if (obj_mesh->vertices_used > (BLACK_SHADES_MAX_VERTICES) || obj_mesh->faces_used > (BLACK_SHADES_MAX_FACES))
        {
            printf("Warning : more than %d vertices or than %d faces may not be supported in the current state of Black Shades !\n", BLACK_SHADES_MAX_VERTICES, BLACK_SHADES_MAX_FACES);
        }

    /* open solid output file for writing in binary mode */
    output_file = fopen(solid_file_path, "wb");
    if (output_file == NULL)
        {
            printf("Error : can't open '%s' for writing !\n", solid_file_path);
            return;
        }

    printf("vertices = %d\n", obj_mesh->vertices_used);
    printf("faces = %d\n", obj_mesh->faces_used);
    vertices_count = obj_mesh->vertices_used;
    faces_count = 0;
    /* we recount the faces as some of them may be quads (they count for two tri faces) */
    for (face_index = 0; face_index < obj_mesh->faces_used; face_index++)
        {
            if (obj_mesh->faces[face_index].is_quad)
                {
                    faces_count+=2;
                }
            else
                {
                    faces_count++;
                }
        }
    solid_write_short(output_file, 1, &vertices_count);
    solid_write_short(output_file, 1, &faces_count);

    for (vertex_index = 0; vertex_index < obj_mesh->vertices_used; vertex_index++)
        {
            solid_write_float(output_file, 1, &obj_mesh->vertices[vertex_index].x);
            /* swap vectors (Z is the up vector in blender) */
            vertex_coordinate = obj_mesh->vertices[vertex_index].z;
            solid_write_float(output_file, 1, &vertex_coordinate);
            vertex_coordinate = -1 * obj_mesh->vertices[vertex_index].y;
            solid_write_float(output_file, 1, &vertex_coordinate);
        }

    pad = 0;
    color = 0.0f;
    for (face_index = 0; face_index < obj_mesh->faces_used; face_index++)
        {
            /* get the matching material */
            current_material = obj_get_material_by_name(obj_mesh, obj_mesh->faces[face_index].texture_name);

            /* TRI */
            if (obj_mesh->faces[face_index].is_quad == 0)
                {
                    /* vertices indexes */
                    vertex_index_short = obj_mesh->faces[face_index].vertex_index[0] - 1;
                    solid_write_short(output_file, 1, &vertex_index_short);

                    vertex_index_short = obj_mesh->faces[face_index].vertex_index[1] - 1;
                    solid_write_short(output_file, 1, &vertex_index_short);

                    vertex_index_short = obj_mesh->faces[face_index].vertex_index[2] - 1;
                    solid_write_short(output_file, 1, &vertex_index_short);

                    /* padding */
                    solid_write_short(output_file, 1, &pad);

                    /* color */
                    if (current_material != NULL)
                        {
                            solid_write_float(output_file, 1, &current_material->diffuse_r);
                            solid_write_float(output_file, 1, &current_material->diffuse_g);
                            solid_write_float(output_file, 1, &current_material->diffuse_b);

                        }
                    else
                        {
                            solid_write_float(output_file, 1, &color);
                            solid_write_float(output_file, 1, &color);
                            solid_write_float(output_file, 1, &color);
                        }
                }
            else
                {
                    /* QUAD */

                    /* quads have to be divided in two triangles */

                    /* vertices indexes */
                    vertex_index_short = obj_mesh->faces[face_index].vertex_index[0] - 1;
                    solid_write_short(output_file, 1, &vertex_index_short);

                    vertex_index_short = obj_mesh->faces[face_index].vertex_index[1] - 1;
                    solid_write_short(output_file, 1, &vertex_index_short);

                    vertex_index_short = obj_mesh->faces[face_index].vertex_index[2] - 1;
                    solid_write_short(output_file, 1, &vertex_index_short);

                    /* padding */
                    solid_write_short(output_file, 1, &pad);

                    /* color */
                    if (current_material != NULL)
                        {
                            solid_write_float(output_file, 1, &current_material->diffuse_r);
                            solid_write_float(output_file, 1, &current_material->diffuse_g);
                            solid_write_float(output_file, 1, &current_material->diffuse_b);

                        }
                    else
                        {
                            solid_write_float(output_file, 1, &color);
                            solid_write_float(output_file, 1, &color);
                            solid_write_float(output_file, 1, &color);
                        }

                    /* vertices indexes */
                    vertex_index_short = obj_mesh->faces[face_index].vertex_index[2] - 1;
                    solid_write_short(output_file, 1, &vertex_index_short);

                    vertex_index_short = obj_mesh->faces[face_index].vertex_index[1] - 1;
                    solid_write_short(output_file, 1, &vertex_index_short);

                    vertex_index_short = obj_mesh->faces[face_index].vertex_index[3] - 1;
                    solid_write_short(output_file, 1, &vertex_index_short);

                    /* padding */
                    solid_write_short(output_file, 1, &pad);

                    /* color */
                    if (current_material != NULL)
                        {
                            solid_write_float(output_file, 1, &current_material->diffuse_r);
                            solid_write_float(output_file, 1, &current_material->diffuse_g);
                            solid_write_float(output_file, 1, &current_material->diffuse_b);

                        }
                    else
                        {
                            solid_write_float(output_file, 1, &color);
                            solid_write_float(output_file, 1, &color);
                            solid_write_float(output_file, 1, &color);
                        }
                }
        }

    fclose(output_file);
}

/* print usage of the command */
void usage(char *command_name)
{
    printf( PROGRAM_NAME " v." PROGRAM_VERSION " :\n"
            "\t" PROGRAM_DESCRIPTION "\n\n"
            "usage:\n"
            "\n"
            "[solid->obj] (with 3 args)\n"
            "\n"
            "\t%s <input_solid_file> <output_obj_file> <output_mtl_file>\n"
            "\n"
            "\tinput_solid_file \t:\ta valid solid mesh file\n"
            "\toutput_obj_file \t:\tname of the output obj file to create\n"
            "\toutput_mtl_file \t:\tname of the output material file to create\n"
            "\n"
            "\t! WARNING output files WILL be OVERWRITTEN !\n"
            "\n"
            "[obj->solid] (with 2 args)\n"
            "\n"
            "\t%s <input_obj_file> <output_solid_file>\n"
            "\n"
            "\tinput_obj_file\t\t:\ta valid obj mesh file\n"
            "\toutput_solid_file\t:\tname of the output solid file to create\n"
            "\n"
            "\t! WARNING output file WILL be OVERWRITTEN !\n"
            "\n"
            , command_name, command_name);
}

int main(int argc, char *argv[])
{
    /* path to the input and output mesh files */
    char solid_file_path[1024];
    char obj_file_path[1024];
    char obj_material_file_path[1024];

    /* obj file line buffer */
    char obj_line_buffer[1024];

    /* obj file key */
    char obj_key[255];

    /* obj current material name */
    char obj_current_material_name[1024];

    /* solid mesh file handle */
    FILE *solid_file = NULL;

    /* obj mesh file handle */
    FILE *obj_file = NULL;

    /* obj material file handle */
    FILE *obj_material_file = NULL;

    /* solid mesh */
    solid_mesh_t *solid_mesh = NULL;

    /* obj mesh */
    obj_mesh_t *obj_mesh = NULL;

    /* obj vertex */
    obj_vertex_t *obj_vertex = NULL;

    /* obj face */
    obj_face_t *obj_face = NULL;

    /* obj material */
    obj_material_t *obj_material = NULL;

    /* number of vertices to read from the solid file */
    short vertex_count = 0;
    /* number of triangles to read from the solid file */
    short triangle_count = 0;

    /* initialize paths with '\0' */
    memset(solid_file_path, '\0', 1024);
    memset(obj_file_path, '\0', 1024);
    memset(obj_material_file_path, '\0', 1024);

    /* init obj file line buffer */
    memset(obj_line_buffer, '\0', 1024);

    /* init obj file key */
    memset(obj_key, '\0', 255);

    /* init current material name */
    memset(obj_current_material_name, '\0', 1024);

    /* if files are specified at command line */
    if (argc == 3) /* OBJ to SOLID mode */
        {
            /* copy files names into corresponding arrays */
            strncpy(obj_file_path, argv[1], 1024);
            strncpy(solid_file_path, argv[2], 1024);
            /* note : material file name will be extracted from the obj file */


            printf("loading '%s'...\n", obj_file_path);

            /* create the obj mesh in memory */
            obj_mesh = obj_mesh_create(obj_file_path);

            /* open obj file */
            obj_file = fopen(obj_file_path, "r");
            if (obj_file == NULL)
                {
                    printf("can't load file '%s' !\n", obj_file_path);
                    exit(2);
                }

            /* parse obj file */
            while ( fgets(obj_line_buffer, 1024, obj_file) != NULL)
                {
                    sscanf(obj_line_buffer, "%s%*[^\n]", obj_key);

                    /* vertex line */
                    if (strcmp(obj_key, "v") == 0)
                        {
                            obj_vertex = obj_add_vertex(obj_mesh);
                            if (obj_vertex != NULL)
                                {
                                    obj_vertex->x = 0;
                                    obj_vertex->y = 0;
                                    obj_vertex->z = 0;
                                    obj_vertex->w = 0;
                                    sscanf(obj_line_buffer, "v %f %f %f %f", &(obj_vertex->x), &(obj_vertex->y), &(obj_vertex->z), &(obj_vertex->w));
                                }
                        }
                    /* face line */
                    else if (strcmp(obj_key, "f") == 0)
                        {
                            obj_face = obj_read_face(obj_line_buffer, obj_mesh);
                            if (strlen(obj_current_material_name) > 0)
                                {
                                    strncpy(obj_face->texture_name, obj_current_material_name, 1024);
                                }
                        }
                    /* use material */
                    else if (strcmp(obj_key, "usemtl") == 0)
                        {
                            sscanf(obj_line_buffer, "usemtl %s", obj_current_material_name);
                        }
                    /* comment */
                    else if (strcmp(obj_key, "#") == 0)
                        {

                        }
                    /* material file */
                    else if (strcmp(obj_key, "mtllib") == 0)
                        {
                            sscanf(obj_line_buffer, "mtllib %s", obj_material_file_path);
                        }
                }

            /* close obj file */
            fclose(obj_file);


            /* has a MTL (material) file been declared in the obj file ? (mtllib directive ?) */
            if (strlen(obj_material_file_path)>0)
                {
                    obj_material_file = fopen(obj_material_file_path, "r");
                    if (obj_material_file == NULL)
                        {
                            printf("Error : can't open material file '%s' for reading !/n", obj_material_file_path);
                        }
                    else
                        {
                            obj_material = NULL;
                            /* parse material file */
                            while ( fgets(obj_line_buffer, 1024, obj_material_file) != NULL)
                                {
                                    sscanf(obj_line_buffer, "%s%*[^\n]", obj_key);
                                    if (strcmp(obj_key, "newmtl") == 0)
                                        {
                                            obj_material = obj_add_material(obj_mesh);
                                            sscanf(obj_line_buffer, "newmtl %s", obj_material->name);
                                            obj_material->ambient_r = 0.0f;
                                            obj_material->ambient_g = 0.0f;
                                            obj_material->ambient_b = 0.0f;
                                            obj_material->diffuse_r = 0.0f;
                                            obj_material->diffuse_g = 0.0f;
                                            obj_material->diffuse_b = 0.0f;
                                            obj_material->specular_r = 0.0f;
                                            obj_material->specular_g = 0.0f;
                                            obj_material->specular_b = 0.0f;
                                            obj_material->specular_coefficient = 0.0f;
                                        }
                                    else if (strcmp(obj_key, "Ka") == 0)
                                        {
                                            if (obj_material != NULL)
                                                {
                                                    sscanf(obj_line_buffer, "Ka %f %f %f", &(obj_material->ambient_r), &(obj_material->ambient_g), &(obj_material->ambient_b));
                                                }
                                        }
                                    else if (strcmp(obj_key, "Kd") == 0)
                                        {
                                            if (obj_material != NULL)
                                                {
                                                    sscanf(obj_line_buffer, "Kd %f %f %f", &(obj_material->diffuse_r), &(obj_material->diffuse_g), &(obj_material->diffuse_b));
                                                }
                                        }
                                    else if (strcmp(obj_key, "Ks") == 0)
                                        {
                                            if (obj_material != NULL)
                                                {
                                                    sscanf(obj_line_buffer, "Ks %f %f %f", &(obj_material->specular_r), &(obj_material->specular_g), &(obj_material->specular_b));
                                                }
                                        }
                                    else if (strcmp(obj_key, "Ns") == 0)
                                        {
                                            if (obj_material != NULL)
                                                {
                                                    sscanf(obj_line_buffer, "Ka %f", &(obj_material->specular_coefficient));
                                                }
                                        }
                                }

                            fclose(obj_material_file);
                        }
                }

            /* create solid file */
            printf("creating solid file...\n");
            obj_mesh_convert_to_solid(obj_mesh, solid_file_path);
            printf("...done !\n");

            /* free data */
            obj_mesh_free(obj_mesh);
        }
    else if (argc == 4) /* SOLID to OBJ mode */
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
            solid_read_short(solid_file, 1, &vertex_count);
            printf("%d vertices to read\n", vertex_count);

            /* read triangles count in solid file */
            solid_read_short(solid_file, 1, &triangle_count);
            printf("%d triangles to read\n", triangle_count);

            /* allocate memory for the solid mesh */
            solid_mesh = solid_mesh_create(solid_file_path, vertex_count, triangle_count);

            /* fill the solid mesh from file */
            solid_read_XYZ(solid_file, vertex_count, solid_mesh->vertices);
            solid_read_textured_triangle(solid_file, triangle_count, solid_mesh->triangles);

            fclose(solid_file);

            /* create obj file */
            printf("creating obj file...\n");
            solid_mesh_convert_to_obj(solid_mesh, obj_file_path, obj_material_file_path);
            printf("...done !\n");

            /* free data */
            solid_mesh_free(solid_mesh);
        }
    else
        {
            usage(argv[0]);
            exit(1);
        }
    return 0;
}

