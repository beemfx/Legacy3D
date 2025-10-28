#include <stdio.h>
#include <memory.h>
#include <malloc.h>

#pragma pack(1)

typedef struct _ms3d_vertex{
	unsigned char  flags;     // SELECTED | SELECTED2 | HIDDEN
	float          vertex[3]; //
	signed char    boneId;    // -1 = no bone
	unsigned char  referenceCount;
}ms3d_vertex;

typedef struct _ms3d_triangle{
	 unsigned short    flags;                // SELECTED | SELECTED2 | HIDDEN
    unsigned short    vertexIndices[3];     //
    float             vertexNormals[3][3];  //
    float             s[3];                 //
    float             t[3];                 //
    unsigned char     smoothingGroup;       // 1 - 32
    unsigned char     groupIndex;           //
}ms3d_triangle;

typedef struct
{
    unsigned char          flags;                              // SELECTED | HIDDEN
    signed char            name[32];                           //
    unsigned short         numtriangles;                       //
    unsigned short*        triangleIndices;      // the groups group the triangles
    signed char            materialIndex;                      // -1 = no material
} ms3d_group_t;

typedef struct _MILKSHAPE_FILE{
	char id[10];
	int  version;

	unsigned short nNumVertices;
	ms3d_vertex*   pVertices;

	unsigned short nNumTriangles;
	ms3d_triangle* pTriangles;

	unsigned short nNumGroups;
	ms3d_group_t*  pGroups;
}MILKSHAPE_FILE;

int main()
{
	MILKSHAPE_FILE msf;
	char szFile[]=".\\blaine\\me_skin.ms3d";
	FILE* fin=NULL;
	int i=0, j=0;

	memset(&msf, 0, sizeof(MILKSHAPE_FILE));

	fin=fopen(szFile, "rb");
	if(!fin)
		return 0;

	printf("Openened \"%s\"\n", szFile);
	//Read the header.
	fread(&msf.id, 1, 10, fin);
	fread(&msf.version, 4, 1, fin);
	printf("id: %s\n", msf.id);
	printf("version: %i\n", msf.version);
	
	//Read the vertices
	fread(&msf.nNumVertices, 2, 1, fin);
	printf("Vertex Count: %i\n", msf.nNumVertices);
	msf.pVertices=malloc(msf.nNumVertices*sizeof(ms3d_vertex));
	for(i=0; i<msf.nNumVertices; i++)
	{
		fread(&msf.pVertices[i], 1, 15, fin);
		printf("Vertex[%i]: f: %i; (%f, %f, %f); b: %i; r: %i\n",
			i+1,
			msf.pVertices[i].flags,
			msf.pVertices[i].vertex[0],
			msf.pVertices[i].vertex[1],
			msf.pVertices[i].vertex[2],
			msf.pVertices[i].boneId,
			msf.pVertices[i].referenceCount);

	}
	fread(&msf.nNumTriangles, 2, 1, fin);
	printf("Triangle Count: %i\n", msf.nNumTriangles);
	msf.pTriangles=malloc(msf.nNumTriangles*sizeof(ms3d_triangle));
	for(i=0; i<msf.nNumTriangles; i++)
	{
		fread(&msf.pTriangles[i], 1, sizeof(ms3d_triangle), fin);
		//if(i>=0 && i<10)
		printf("Triangle[%i]: f: %i; (%i, %i, %i); [other traingle information]; sg: %i; g: %i\n",
			i+1,
			msf.pTriangles[i].flags,
			msf.pTriangles[i].vertexIndices[0],
			msf.pTriangles[i].vertexIndices[1],
			msf.pTriangles[i].vertexIndices[2],
			msf.pTriangles[i].smoothingGroup,
			msf.pTriangles[i].groupIndex);
	}

	fread(&msf.nNumGroups, 2, 1, fin);
	printf("Groups: %i\n", msf.nNumGroups);
	msf.pGroups=malloc(sizeof(ms3d_group_t)*msf.nNumGroups);
	for(i=0; i<msf.nNumGroups; i++)
	{
		char szTemp[32];
		fread(&msf.pGroups[i].flags, 1, 1, fin);
		fread(&msf.pGroups[i].name, 1, 32, fin);
		printf("Name: %s\n", msf.pGroups[i].name);
		fread(&msf.pGroups[i].numtriangles, 2, 1, fin);
		printf("Triangles: %i\n", msf.pGroups[i].numtriangles);
		msf.pGroups[i].triangleIndices=malloc(msf.pGroups[i].numtriangles*2);
		for(j=0; j<msf.pGroups[i].numtriangles; j++)
		{
			fread(&msf.pGroups[i].triangleIndices[j], 2, 1, fin);
			//printf("Triangle[%i]: %i\n", j+1, msf.pGroups[i].triangleIndices[j]);
		}
		fread(&msf.pGroups[i].materialIndex, 1, 1, fin);
		printf("mi: %i\n", msf.pGroups[i].materialIndex);
	}


	fclose(fin);
	return 0;
}