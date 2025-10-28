#pragma warning(disable:4996)

#include "../../game_engine/ext_libs/Base64Conv.h"
#include <stdio.h>
#include <string.h>

enum FLOAT_FORMAT: int{FF_TEXT, FF_BASE64};

void LMSHtoEMESH(FILE* fin, FILE* fout, FLOAT_FORMAT ff);



int main(int argc, char *argv[], char *envp[])
{
	FILE *fin, *fout;	
	FLOAT_FORMAT ff = FF_BASE64;
	
	char szIn[1024];
	char szOut[1024];
	
	printf("Legacy Mesh to Emergence Mesh Converter Utility 1.00\n");
	if(argc>1)
	{
		sprintf(szIn, argv[1]);
	}
	else
	{
		printf("Usage: LMESHtoEMESH.exe \"lmesh file\" [\"emesh file\" [text|base-64]]\n");
		return 0;
	}
	
	if(argc>2)
	{
		sprintf(szOut, argv[2]);
	}
	else
	{
		sprintf(szOut, "%s.emesh", argv[1]);
	}
	
	
	
	if(argc>3)
	{
		if(!stricmp("text", argv[3]))
			ff=FF_TEXT;
		else if(!stricmp("base=64", argv[3]))
			ff=FF_BASE64;
	}
	
	fin = fopen(szIn, ("rb"));
	fout = fopen(szOut, ("wb"));
	
	LMSHtoEMESH(fin, fout, ff);
	
	fclose(fin);
	fclose(fout);
}

void LMSHtoEMESH(FILE* fin, FILE* fout, FLOAT_FORMAT ff)
{
	//Need just three base64 converters.
	CBase64Conv b64[3];
	
	typedef float Vertex[8];
	typedef unsigned short Triangle[3];
	
	
	char szTL[1024];
	char szString[1024];
	unsigned long nTDW=0;
	float vertex[8];
	unsigned short triangle[3];
	szTL[0]=0;
	
	
	#define READDW fread(&nTDW, 1, 4, fin);
	#define READSTR(size) fread(szString, 1, size, fin);
	#define READVTX fread(vertex, 1, sizeof(float)*8, fin);
	#define READTRI fread(triangle, 1, 2*3, fin);
	
	#define WRITE fwrite(szTL, 1, strlen(szTL), fout);
	
	//Write the xml header.
	strcpy(szTL, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
	WRITE
	
	sprintf(szTL, "<emesh\n");
	WRITE
	
	/////////////
	///Header:///
	/////////////
	
	READDW //ID
	if(nTDW!=0x48534D4C)
	{
		return;
	}
	
	//May not really need the version information.
	READDW
	nTDW++;
	sprintf(szTL, "\tversion=\"%u\"\n", nTDW);
	WRITE
	
	READSTR(32)
	sprintf(szTL, "\tname=\"%s\"\n", szString);
	WRITE
	
	READDW
	int vertexes = nTDW;
	sprintf(szTL, "\tvertexes=\"%u\"\n", nTDW);
	WRITE
	
	READDW
	int triangles = nTDW;
	sprintf(szTL, "\ttriangles=\"%u\"\n", nTDW);
	WRITE
	
	READDW
	int meshes=nTDW;
	sprintf(szTL, "\tmeshes=\"%u\"\n", nTDW);
	WRITE
	
	READDW
	int materials=nTDW;
	sprintf(szTL, "\tmaterials=\"%u\"\n", nTDW);
	WRITE
	
	READDW
	int bones=nTDW;
	sprintf(szTL, "\tbones=\"%u\"\n", nTDW);
	WRITE
	
	if(FF_TEXT==ff)
		sprintf(szTL, "\tformat=\"text\"\n");
	else
		sprintf(szTL, "\tformat=\"base64\"\n");
	WRITE
	
	sprintf(szTL, ">\n");
	WRITE
	
	
	/////////////////////
	/// Read Vertexes ///
	////////////////////
	
	Vertex* pVerts = new Vertex[vertexes];
	for(int i=0; i<vertexes; i++)
	{
		READVTX
		memcpy(pVerts[i], vertex, sizeof(vertex));
	}
	
	//////////////////////
	/// Read Triangles ///
	//////////////////////
	Triangle* pTris = new Triangle[triangles];
	for(int i=0; i<triangles; i++)
	{
		READTRI
		memcpy(pTris[i], triangle, sizeof(triangle));
	}
	
	////////////////////////
	// Read vertex bones ///
	////////////////////////
	unsigned long* pBoneList = new unsigned long[vertexes];
	for(int i=0; i<vertexes; i++)
	{
		READDW
		pBoneList[i]=nTDW;
	}
	
	
	for(int i=0; i<bones; i++)
	{
		READSTR(32)
		sprintf(szTL, "\t<bone id=\"%u\" name=\"%s\"/>\n", i+1, szString);
		WRITE
	}
	
	sprintf(szTL, "\n");
	WRITE
	
	for(int i=0; i<meshes; i++)
	{
		READSTR(32)
		sprintf(szTL, "\t<mesh id=\"%u\" name=\"%s\" ", i+1, szString);
		WRITE
		
		READDW
		sprintf(szTL, "first_triangle=\"%u\" ", (nTDW/3)+1);
		WRITE
		
		READDW
		sprintf(szTL, "triangles=\"%u\" ", nTDW);
		WRITE
		
		READDW
		sprintf(szTL, "material=\"%u\"/>\n", nTDW+1);
		WRITE
		
	}
	
	sprintf(szTL, "\n");
	WRITE
	
	for(int i=0; i<materials; i++)
	{
		READSTR(32)
		sprintf(szTL, "\t<material id=\"%u\" name=\"%s\" ", i+1, szString);
		WRITE
		READSTR(260)
		sprintf(szTL, " texture=\"%s\"/>\n", szString);
		WRITE
	}
	
	sprintf(szTL, "\n");
	WRITE
	
	float bb[6];
	fread(bb, 1, sizeof(float)*6, fin);
	if(FF_TEXT==ff)
	{
		sprintf(szTL, "\t<bounds min=\"%g %g %g\" max=\"%g %g %g\"/>\n",
			bb[0], bb[1], bb[2], bb[3], bb[4], bb[5]);
	}
	else
	{
		unsigned long size=0;
		b64[0].Load((b64_byte*)&bb[0], sizeof(float)*3);
		b64[1].Load((b64_byte*)&bb[3], sizeof(float)*3);
		//char* szMin = ::ToBase64(&size, &bb[0], sizeof(float)*3);
		//char* szMax = ::ToBase64(&size, &bb[3], sizeof(float)*3);
		sprintf(szTL, "\t<bounds min=\"%s\" max=\"%s\"/>\n",
			b64[0].GetBase64String(NULL), b64[1].GetBase64String(NULL));
			
		//::FreeToBase64(szMin);
		//::FreeToBase64(szMax);
	}
	WRITE
	
	//////////////////////////
	/// Write the Vertexes ///
	//////////////////////////
	sprintf(szTL, "\n");
	WRITE
	
	for(int i=0; i<vertexes; i++)
	{
		if(FF_TEXT==ff)
		{
			sprintf(szTL, "\t<vertex id=\"%u\" position=\"%g %g %g\" normal=\"%g %g %g\" tex1=\"%g %g\" bone = \"%u\"/>\n",
				i+1, 
				pVerts[i][0], pVerts[i][1], pVerts[i][2], 
				pVerts[i][3], pVerts[i][4], pVerts[i][5], 
				pVerts[i][6], pVerts[i][7], 
				pBoneList[i]+1);
		}
		else
		{
			unsigned long size=0;
			b64[0].Load(&pVerts[i][0], sizeof(float)*3);
			b64[1].Load(&pVerts[i][3], sizeof(float)*3);
			b64[2].Load(&pVerts[i][6], sizeof(float)*2);
			sprintf(szTL, "\t<vertex id=\"%u\" position=\"%s\" normal=\"%s\" tex1=\"%s\" bone = \"%u\"/>\n",
				i+1,
				b64[0].GetBase64String(NULL),
				b64[1].GetBase64String(NULL),
				b64[2].GetBase64String(NULL),
				pBoneList[i]+1);
		}
		WRITE
	}
	
	///////////////////////
	/// Write Triangles ///
	///////////////////////
	
	sprintf(szTL, "\n");
	WRITE
	
	for(int i=0; i<triangles; i++)
	{
		sprintf(szTL, "\t<triangle id=\"%u\" v1=\"%u\" v2=\"%u\" v3=\"%u\"/>\n",
			i+1, pTris[i][0]+1, pTris[i][1]+1, pTris[i][2]+1);
		WRITE
	}
	
	sprintf(szTL, "</emesh>");
	WRITE
	
	
	delete[]pVerts;
	delete[]pBoneList;
	return;
}
