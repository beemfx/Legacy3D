/* lx_sys.h - XML Loading routines.

	Copyright (c) 2007 Blaine Myers

	Notes: The XML loading routines are used to load various
	.xml files and returns the information contained in 
	those files in a more computer usable data structure.
	Each XML loader comes in two parts.  LX_Load#TYPE# which
	loads the specified file and returns computer 
	understandable information about the file.  And
	LX_Destroy#TYPE# which frees any memory used in loading
	the file.
	
	XML Loading routines only convert the data in the files
	into usable information.  They do not actually load the
	specified data into the game engine.
*/
#include "common.h"
#include "lf_sys2.h"
#include "lg_xml.h"
#include "lg_err.h"

#define LX_START_TAG {if(0){}
#define LX_END_TAG else {Err_Printf("WARNING: %s is not recognized as a valid tag.", name);}}

#define LX_TAG(tag_name) else if(stricmp(name, #tag_name)==0)

//Helper macros
#define LX_START_ATTR for(lg_dword i=0; atts[i]; i+=2){if(0){}
#define LX_END_ATTR }

#define LX_ATTR_DWORD(dest, name) else if(stricmp(atts[i], #name)==0){dest=atoi(atts[i+1]);}
#define LX_ATTR_FLOAT(dest, name) else if(stricmp(atts[i], #name)==0){dest=(lg_float)atof(atts[i+1]);}
#define LX_ATTR_STRING(dest, name, size) else if(stricmp(atts[i], #name)==0){LG_strncpy(dest, atts[i+1], size);}
#define LX_ATTR_FLOAT3(dest, name) else if(stricmp(atts[i], #name)==0){ \
	XML_Char* szTok=strtok((XML_Char*)atts[i+1], ", ");      \
	((lg_float*)dest)[0]=szTok?(lg_float)atof(szTok):0.0f;   \
	szTok=strtok(LG_NULL, ", ");                             \
	((lg_float*)dest)[1]=szTok?(lg_float)atof(szTok):0.0f;   \
	szTok=strtok(LG_NULL, ", ");                             \
	((lg_float*)dest)[2]=szTok?(lg_float)atof(szTok):0.0f;   \
}
#define LX_MISC_ATTR(name) else if(stricmp(atts[i], #name)==0){
#define LX_END_MISC_ATTR }



#define LX_MAX_NAME 255
typedef lg_char lx_name[LX_MAX_NAME+1];

typedef struct _lx_ent_template
{
	lg_dword nID;
	lg_path  szScript;
}lx_ent_template;

lx_ent_template* LX_LoadEntTemplate(const lg_path szFilename, lg_dword* pCount);
lg_void LX_DestroyEntTemplate(lx_ent_template* pTemplate);

typedef struct _lx_object_mode{
	lx_name szName;
}lx_object_mode;

/*
class CLXLoader
{
public:
	CLXLoader();
	~CLXLoader();
	
	lg_void Load(lg_path szXMLFile);
};
*/

lg_bool LX_Process(const lg_path szXMLFile,
	XML_StartElementHandler startH,
	XML_EndElementHandler endH,
	XML_CharacterDataHandler charDataH,
	lg_void* pData);

//LX_OBJECT: XML Loading for objects (aka entities).

//The LX_OBJ_PHYS_ flags are set in lx_object::pPhysFlags
//and tell information about how the object should be
//created physics wise.
#define LX_OBJ_PHYS_AUTO_CENTER     0x00000001
/*
#define LX_OBJ_PHYS_SHAPE_CAPSULE   0x00000002
#define LX_OBJ_PHYS_SHAPE_BOX       0x00000004
#define LX_OBJ_PHYS_SHAPE_SPHERE    0x00000008
#define LX_OBJ_PHYS_SHAPE_CYLINDER  0x00000010
*/
#define LX_OBJ_PHYS_BOUNDS_SKEL     0x00000020
#define LX_OBJ_PHYS_BOUNDS_MESH     0x00000040

typedef struct _lx_object{
	lf_path  szModelFile;
	lg_dword nNumSkels;
	lf_path* pSkelFiles;
	lx_name* pSkelMeshes;
	lx_name  szAiFunction;
	
	//Physics stuff.
	lg_float fMass;
	//lg_float fXC, fYC, fZC;
	lg_float fMassCenter[3];
	lg_dword nPhysFlags;
	lg_dword nShape;
	//lg_float fScale;
	lg_float fHeight;
	lg_float fRadius;
	lg_float fShapeOffset[3];
	
	//Modes
	lg_dword nModeCount;
	lx_object_mode* pModes;
}lx_object;

/* LX_LoadObject

	PRE: File System Initialized.  szXMLFile should point to
		a properly declared "object" xml file.
	POST: A new lx_object is created based on the specifications
		in the xml file.  If null is returned it means there was
		some kind of error.  Though if an actual object is returned
		it doesn't necessarily mean there was no error.  Check
		the console to see if an error occured while loading.
		That said, just because there was no errors doesn't mean
		a valid object can get created off the information.  It
		must contain valid files, mesh references and so forth.
		See the declaration of the lx_object structure for details
		on what is returned.
*/
lx_object* LX_LoadObject(const lf_path szXMLFile);

/* LX_DestroyObject

	PRE: pObject != null and should have been created by
		LX_LoadObject.  Any pointer returned by LX_LoadObject
		should be deleted using this method.
	POST: The memory associated with pObject is freed.  pObject
		should no longer be used.
*/
lg_void LX_DestroyObject(lx_object* pObject);


/* The level loading XML, gets all information to load
	a level.
*/
typedef struct _lx_level_object{
	lg_path szObjFile;
	lg_float fPosition[3];
}lx_level_object;

typedef struct _lx_level{
	lg_path szMapFile;
	lg_path szSkyBoxFile;
	lg_path szSkyBoxSkin;
	lg_path szMusicFile;
	lg_dword nObjCount;
	lg_dword nActorCount;
	lx_level_object* pObjs;
	lx_level_object* pActors;
}lx_level;

/* PRE: See LoadObject for example.
	POST: See LoadObject for example.
*/
lx_level* LX_LoadLevel(const lg_path szXMLFile);

/* PRE: See DestroyObject for example.
	POST: See DestroyObject for example.
*/
lg_void LX_DestroyLevel(lx_level* pLevel);



/***********************
*** Material Loading ***
***********************/

typedef struct _lx_mtr{
	lg_path szTexture;
	lg_path szFx;
}lx_mtr;

/* PRE: pMtr should be a MtrItem.
	POST: If LG_TRUE was returend then
	pMtr contains a valid material, if not
	then the material is invalid.
*/
lg_bool LX_LoadMtr(const lg_path szFilename, lx_mtr* pMtr);

/* PRE: N/A
	POST: Loads teh skin, returns false if it can't.
*/
lg_bool LX_LoadSkin(const lg_path szFilename, void* pSkin);

/************************
*** Mesh Tree loading ***
************************/
#include "lm_mesh_tree.h"

typedef struct _lx_mesh_node{
	lg_string szName;
	lg_path   szMeshFile;
	lg_path   szSkinFile;
	lg_path   szDefSkelFile;
	lg_dword  nParentNode;
	lg_string szParentJoint;
}lx_mesh_node;

typedef struct _lx_mesh_tree_data{
	lg_string    szName;
	lg_dword     nNodeCount;
	lg_dword     nSkelCount;
	lg_bool      bYaw180;
	lg_float     fScale;
	lx_mesh_node Nodes[CLMeshTree::MT_MAX_MESH_NODES];
	lg_path      szSkels[CLMeshTree::MT_MAX_SKELS];
}lx_mesh_tree_data;

/* PRE: szFilename is the file to load.
	POST: If true is returned then pOut will be
	full of infor about the tree.  If not the data
	is invalid.  The data doesn't need to be deallocated.
*/
lg_bool LX_LoadMeshTree(const lg_path szFilename, lx_mesh_tree_data* pOut);