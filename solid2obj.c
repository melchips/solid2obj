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
#define PROGRAM_VERSION "0.1a"
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

void solid_mesh_convert_to_obj(solid_mesh_t *solid_mesh, char *output_file_path)
{
    FILE *output_file = NULL;
    int vertex_index = 0;
    int triangle_index = 0;

    if (solid_mesh == NULL)
    {
        printf("Error : solid mesh is NULL in function solid_mesh_convert_to_obj !\n");
        return;
    }
   
    output_file = fopen(output_file_path, "w");
    if (output_file == NULL)
    {
        printf("Error : can't open '%s' for writing !\n", output_file_path);
        return;
    }

    fprintf(output_file, "# exported from Blackshade's solid mesh file '%s'\n", solid_mesh->filename);
    for(vertex_index=0;vertex_index<solid_mesh->vertex_count;vertex_index++)
    {
        fprintf(output_file, "v %f %f %f 1.0\n",
                solid_mesh->vertices[vertex_index].x,
                solid_mesh->vertices[vertex_index].y,
                solid_mesh->vertices[vertex_index].z);
    }

    for(triangle_index=0;triangle_index<solid_mesh->triangle_count;triangle_index++)
    {
        fprintf(output_file, "f %hu %hu %hu\n",
                solid_mesh->triangles[triangle_index].vertex[0] + 1,
                solid_mesh->triangles[triangle_index].vertex[1] + 1,
                solid_mesh->triangles[triangle_index].vertex[2] + 1
              );
    }

    fclose(output_file);
}

/* print usage of the command */
void usage(char *command_name)
{
    printf( PROGRAM_NAME " v." PROGRAM_VERSION " :\n"
            "\t" PROGRAM_DESCRIPTION "\n\n"
            "usage:\n"
            "%s <input_file> <output_file>\n"
            "\tinput file\t:\ta valid solid mesh file\n"
            "\toutput_file\t:\tname of the output obj file to create\n"
            "\t\t\t\tWARNING output file WILL be OVERWRITTEN !\n"
            , command_name);
}


int main(int argc, char *argv[])
{
    /* path to the input and output mesh files */
    char solid_file_path[1024];
    char obj_file_path[1024];

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

    /* if files are specified at command line */
    if (argc > 2)
    {
        /* copy files names into corresponding arrays */
        strncpy(solid_file_path, argv[1], 1024);
        strncpy(obj_file_path, argv[2], 1024);

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
        solid_mesh_convert_to_obj(solid_mesh, obj_file_path);
        printf("...done !\n");

        /* free data */
        solid_mesh_free(solid_mesh);
    } else {
        usage(argv[0]);
        exit(1);
    }
    return 0;
}

