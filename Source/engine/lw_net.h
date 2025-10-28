#ifndef __LW_NET_H__
#define __LW_NET_H__

#include "lg_list_stack.h"

enum CLNT_CONNECT_TYPE{
	CLNT_CONNECT_DISC=0, //Not connected.
	CLNT_CONNECT_TCP,  //Connected using TCP.
	CLNT_CONNECT_UDP,
	CLNT_CONNECT_LOCAL,//Connected to local server.
};

//Server to client commands:
typedef enum _SRV_TO_CLNT_CMDS{
	//Server shutdown:
	//Params: None.
	//Notes: This is broadcast to all clients
	//before the server shuts down.  After this message
	//has been broadcast, the client should assume that
	//it should disconnect.
	STCC_SHUTDOWN=0,
	
	//Client Disconnect:
	//Params: None.
	//Notes: When a client is sent this message it should
	//assume that it will no longer receive updates from
	//the server, and as far as the server is concerned
	//the client is no longer connected.
	STCC_DISCONNECT,
	
	//Entity Move:
	//Params: Information about the entity.
	//Notes: This is broadcast to clients when something
	//about an entity changes.  It will be followed by
	//additional information about what changed.  Note, 
	//that just because an entity changes, doesn't mean
	//that this message is sent to the client.  The server
	//will usually only send relevant changes to a client
	//so if an object is too far away to matter, information
	//about it will not be sent.
	STCC_ENT_MOVE,
	
	//Entity create:
	//Params: ID of the entity that was created, and
	//information about the entity.
	//Notes: Indicates that an entity was created, all
	//clients need to receive this information, and need
	//to reply with confirmation that the entity was
	//created.
	STCC_ENT_CREATE,
	
	//Entity Destroy:
	//Params: ID of the entity to be destroyed.
	//Notes: Indicates that an entity was destroyed, and
	//therefore the client no longer needs to rasterize the entity.
	STCC_ENT_DESTROY,
	
	//Entity Info:
	//Params: Ent ID, information about ent.
	//Notes: Generally sent because the client asked for an
	//update on the information about an entity, similar information
	//is sent as compared to STCC_ENT_MOVE, but when this command
	//is specified, it is possible that the entity specified,
	//no longer exists, or is a different type of entity.
	STCC_ENT_INFO,
	
	//Map change:
	//Params: New map information.
	//Notes: Typically if the map changes, then all entities
	//will change as well.  Either way, the client is going
	//to take time to load the new map, so there will be lag
	//no matter what, so typically if a map change is specified
	//then STCC_ENT creates and destroys will be called as
	//well.
	STCC_MAP_CHANGE,
	
	//Level chagne:
	//Params: N/A
	//Notes: Unlike map change, when the client receives this
	//command it knows that it should destroy all entities, and
	//expect to receive information about all entities within
	//the map.
	STCC_LEVEL_CHANGE
	
}SRV_TO_CLNT_CMDS;

//Client to server commands:
typedef enum _CLNT_TO_SRV_CMDS{
	//Disconnect:
	//Params: None.
	//Notes: Indicates that the client is disconnecting from
	//the server, and will no longer receive or send data.
	CTSC_DISCONNECT=0,
	
	//Input:
	//Params: ID of the entity to receive input, input data.
	//Notes: Indicates that the client wants the entity it
	//owns to process input.
	CTSC_INPUT,
	
	//Request Ent Info:
	//Params: ID of the ent, info is desired for.
	//Notes: Sends a request for the info about the ent,
	//a STCC_ENT_INFO is expected.
	CTSC_REQUEST_ENT_INFO
	
}CLNT_TO_SRV_CMDS;

//Command stack:
struct CLWCmdItem: public CLListStack::LSItem
{
	lg_byte Command;
	lg_word Size;        //Parameter list size.
	lg_byte Padding[1];
	
	lg_byte Params[128];
};

struct STCC_ENT_DESTROY_INFO{
	lg_dword nEntID;
};

struct STCC_ENT_MOVE_INFO{
	lg_dword nEntID;
	ml_mat   matPos;
	ml_aabb  aabbBody;
	lg_dword nFlags[2];
	lg_dword nAnimFlags[2];
	ml_vec3  v3Vel;
	ml_vec3  v3Look[3];
};

struct CTSC_INPUT_INFO{
	lg_dword nEntID;
	lg_dword nCmdsActive[1];
	lg_dword nCmdsPressed[1];
	lg_float fAxis[2];
};

#endif __LW_NET_H__