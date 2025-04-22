#include "readfile.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#define MAX_MATERIAL_COUNT 80

MeshData readfile(const char* FileName) {
    Vec3 ambient[100], diffuse[100], specular[100];  // Assuming max material count = 100
    float shine[100];
    int material_count, color_index[3], i;
    char ch;
    
    FILE* fp = fopen(FileName, "r");
    if (fp == nullptr) {
        std::cerr << "ERROR: Unable to open TriObj [" << FileName << "]!\n";
        exit(1);
    }

    // Skip object name
    fscanf(fp, "%c", &ch);
    while (ch != '\n') fscanf(fp, "%c", &ch);

    int NumTris;
    fscanf(fp, "# triangles = %d\n", &NumTris);  // Read # of triangles
    fscanf(fp, "Material count = %d\n", &material_count);  // Read material count

    for (i = 0; i < material_count; i++) {
        fscanf(fp, "ambient color %f %f %f\n", &ambient[i].x, &ambient[i].y, &ambient[i].z);
        fscanf(fp, "diffuse color %f %f %f\n", &diffuse[i].x, &diffuse[i].y, &diffuse[i].z);
        fscanf(fp, "specular color %f %f %f\n", &specular[i].x, &specular[i].y, &specular[i].z);
        fscanf(fp, "material shine %f\n", &shine[i]);
    }

    fscanf(fp, "%c", &ch);
    while (ch != '\n') fscanf(fp, "%c", &ch);  // Skip documentation line

    std::vector<Triangle> Tris(NumTris);  // Use vector to handle dynamic size

    for (i = 0; i < NumTris; i++) {
        fscanf(fp, "v0 %f %f %f %f %f %f %d\n", &Tris[i].v0.x, &Tris[i].v0.y, &Tris[i].v0.z,
               &Tris[i].norm[0].x, &Tris[i].norm[0].y, &Tris[i].norm[0].z, &color_index[0]);

        fscanf(fp, "v1 %f %f %f %f %f %f %d\n", &Tris[i].v1.x, &Tris[i].v1.y, &Tris[i].v1.z,
               &Tris[i].norm[1].x, &Tris[i].norm[1].y, &Tris[i].norm[1].z, &color_index[1]);

        fscanf(fp, "v2 %f %f %f %f %f %f %d\n", &Tris[i].v2.x, &Tris[i].v2.y, &Tris[i].v2.z,
               &Tris[i].norm[2].x, &Tris[i].norm[2].y, &Tris[i].norm[2].z, &color_index[2]);

        fscanf(fp, "face normal %f %f %f\n", &Tris[i].face_normal.x, &Tris[i].face_normal.y, &Tris[i].face_normal.z);

        Tris[i].Color[0] = static_cast<unsigned char>(255 * diffuse[color_index[0]].x);
        Tris[i].Color[1] = static_cast<unsigned char>(255 * diffuse[color_index[0]].y);
        Tris[i].Color[2] = static_cast<unsigned char>(255 * diffuse[color_index[0]].z);
    }

    fclose(fp);

    // Store result in MeshData
    MeshData r;
    r.tris = Tris;  // Move vector to avoid extra copying
    r.numTris = NumTris;

    return r;
}