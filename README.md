# Legacy 3D Engine
Legacy 3D Engine (c) 2008 Beem Media. All rights reserved.

A primitive 3D engine.

## Building
This version is not buildable since it depends on 3rd Party libraries that are
not included in this repo. I do plan on getting it buildable, but it's mostly
for reference.

## History
Originated: February 14, 2006

Scrapped: 2009

### Original Readme.txt
<pre>
Legacy 3D Engine
================
Engineered by Blaine Myers

(c) 2008 Beem Media

The Legacy Engine was my first 3D engine. It's not very good and is not meant
to be representative of my skills as an engineer. The engine was scrapped in 
2008. It was developed before I knew anything about software engineering and so
it has some really bad code. There are some good ideas in it, and it had some 
interesting tools (albeit unecessary), namely the LMEdit tool and various 
plugins.

It also featured a math library written in assembly (but it was basically a
less featured copy of d3dx9 math).

The code never really came along far enough to even make a basic game demo in
it. It supported various physics engines, the primary was meant to be PhysX
though that is no longer supported by this source (since I'm not even sure what
version of PhysX it was originally built with.) The provided code works with
the Newton Game Dynamics physics engine and the in-house physics engine (though
being that there is no gameplay, everything seems to be pretty chaotic in the
demo).

I'm  keeping the code for novelty purposes, but it has been surpassed by the
Emergence Engine (which is not open source at this time).

The executables in the base directory work and were built with Visual Studio
2015 (except for LPaKager) so they'll require a recent version of the Visual
C++ runtimes installed. The executable for LPaKager doesn't have source code
any more, and it is the only version of the software that will open the game
data in baseuncmp (besides the game client itself).

The Scrapped directory has old version or obsolete code (or stuff I didn't want
to build for this distribution). I didn't know about source control when I wrote
this engine so I originally dumped old files into that directory.
</pre>

### Original DevelopmentLog.txt
<pre>
Legacy3D Engine copyright (c) 2006, Blaine Myers.

To Do:
	Finish the project, of course.  Seriously this to do list is just
	for things I want to impliment that I don't want to forget about.

Notes:
	Organization:
	The Legacy 3D proejct is organized using nameing conventions for
	filenames and types.

	File:  Files are named using the following conventions: 
	*prefix*_*suffix*.*ext* (eg. lv_sys.c).  The prefix represents the
	game system that uses the functions in that file, the suffix represents
	a particular part of that system.  The extension is simply the type
	of file.  Header files should match the name of the source file,
	except with a .h extension of course, with the exception that
	a *prefix*_sys.h file can be the header for any source files with
	the same prefix if desired.

	Current systems and prefixes:

	lg_: The Legacy Game System, sort of the entry point that controls
		all other systems.
	win_: The Windows system, deals with all windows specific API tasks.
	lv_: The Legacy Game Video System, deals with things related to 
		displaying graphics in the game.  Direct3D etc.
	ls_: The Legacy Game Sound System, deals with  things related to the
		audio in the game.
	lf_: The Legacy Game File System, deals with disk input and output
		for the game, as well as Legacy Archive (lpk) management.
	lc_: The Legacy Console System (contained in lc_sys.dll).  Deals with
		functions relating to the console and the cvarlist.

	The suffix is used to specify the usage of the functions contained
	within it.  The standard suffix _sys contains the basic operations
	for that particular system, and may contain all of the functions.
	For larger systems the suffix generally is for a particular
	component (eg. The font system for the video is contained in
	lv_font.c).

	Functions:  Functions are typically prefixed with the system prefix
	in capital letters (eg. LG_Init() for an lg_ function).  Exceptions
	are functions that are used to manipulate handles (eg. CVar functions
	are prefixed CVar_, console Con_, Files File_, Font Font_, etc.).

	Types and Generic Functions and Macros:  All of the common stuff
	is contained in common.c and common.h this contains common functions
	and all of the types used in the game.  Types are prefixed with
	and L_ followed by a short description of the type in lower case
	letters. (eg. lg_dword for dwords (unsigned long).  Macros and
	functions are also prefixed with a L_, but not necessarily in 
	lower or upper case.  (I do want to change this for easier reading).
		

2/14/2006 - Valentines day, and I love computer programming, so what
	better day to start this project than today.  I began simply by
	initializing Direct3D.  For now everything is pretty much just
	dummy stuff.  I decided to call the engine Legacy3D, because it
	will be used to power the "Legacy of Heaven & Hell" game.  It's been
	a while since I've programmed so I've started from scratch, using
	D3DDemo as an example so that I can get used to Direct3D again.
	My next plan is to reimpliment the console, there are a few things
	that I wanted to change from the GFX3D9 version of the BeemConsole
	so I'm not just strait out bringing it over.

3/2/2006 - For the past couple of weeks I have been working on the console
	which is called legacy console.  It is contained in the file lconsole.dll.
	I got the console working pretty good so I finally started working on
	legacy3d again.  Now the game loads variables from a config file, and from
	there it sets up Direct3D using that information.  Right now the way it is
	initialized works, but I think there should probably be less cvars, as
	some of the variables are probably things that don't need to be changed
	for a game.  And some of them are just too complicated to understand.
	It is doing pretty good so far though, so I'm pretty satisfied, in a few
	more days we might get to the point that I'm actually rendering something.
	But before that I'll need to develop 2D so that I can actually see the
	console.  The console still needs a cmdlist function though.

3/3/2006 - Redid some work on the console, and changed a lot of the way the
	game works, to try to make the code less ugly.  Still aren't rendering
	anything yet, hopefully I will be soon.  I don't want to start rendering
	untill I'm sure all the default stuff is pretty stable.  Also I still need
	the game to run some checks on the configuration settings to make sure they
	are valid.

3/6/2006 - I've put quite a bit of work into the basic setup of the game.  I
	reworked a lot of functions, to be easier to read, and to be more
	consistent.  I also put a lot of D3D checking to make sure that the 
	specified modes are allowed on the graphics processor.  Also fixed
	a few bugs in the console.  Still aren't rendering anything.  Need to
	develop a 2D font engine.

3/17/2006 - I've put quite a bit of work into the game basics.  Some sound
	initialization stuff is up and running, and the game plays a basic test
	sound.  Also I did develop a 2d image engine, and a font engine, but the
	font engine was insanely slow and looked bad, so I used ID3DXFont instead,
	along with the 2D image engine and font engine, I developed the basic
	graphical console, and I did some new art for the console background.
	I've built several more console functions based around different debug
	features of the game.  Most impressively, I've built a pak file format
	for the game engine.  It is a simple format with no compression, and
	no encryption.  I made a program called larchiver, that will build
	an archive out of a directory tree.  I've put all the test images and
	sounds into archives to test out how it works with the system, and so
	far it works great so I'm satisfied.  Still no actual 3D rendering being
	done.  Also I interviewed at UPS, so I may have a job soon enough.

3/21/2006 - Work quite a bit more on the file system, or at least on the
	LPK utilities.  LPK is the fileformat for the supported archives in
	Legacy3D it stands for Legacy Package (I guess).  Anyway I made the
	warchiver.exe utility, which is windows based and it will allow the
	user to select a direcory tree to archive, and an output file to 
	arhive to.  I learned some new stuff while developing warchiver, for
	one it uses the progress bar, and for two it uses multithreading (which
	is my first time programming multithreaded stuff).  I've also read
	some more, so the development process should pick up here shortly.

3/31/2006 - I now work for UPS so I won't have as much time to work on
	Legacy.  Today, I reworked a little bit of the audio system, and it
	now supports the OGG (Ogg Vorbis) file format.

7/2/2006 - Well it has been a while since I have written in this log so
	I thought I would write a little bit.  Well I have played a little
	around with light mapping so I think the final map format will use
	light mapping (the map format is a ways away though).  I have also
	begun development of the Legacy 3D Model format (extension L3DM), and
	I have written a plugin for Milkshape 3D.  Right now the model format
	is pretty raw, but I will work on getting it solid and optimized.  Also
	I have been workig out lots of kinks and preparing the engine for
	future features.  I want to rework the file system a little so that
	directories can be changed and archive files can be accessed using
	the relative things (such as . and .. eg. base\..\mod\textures\ type 
	of thing.

7/14/2006 - Just to update I have found a couple of bugs in the code that
	I have repaired.  Also the model format is coming along nicely.  I also
	developed a texture viewer application that will show different mip
	map sublevels and the alpha channel and other cool features for the TGA
	file format.  I'm considering adding BMP support to the TGA library.  It
	wouldn't necessarily be that difficult to convert a BMP file to a TGA
	file then call all the same functions on it.
	
9/16/2006 - Well school has taken a lot of my time, but I have worked a lot
	on the texture viewer I was talking about, and BMPs are almost fully
	supported, with the exception of RLE BMPs.  Also I am now developing
	using VC++ 2005, so it has been a bit different.  I want to finnish up
	the lg_math library and I am planning on using SIMD-x86 as a template
	for 3DNOW, MMX, SSE, etc instructions, but I plan on writing the library
	for NASM so it will be more portable.

2/4/2007 - Yes I have still been working on the project.  The Legacy Mesh and
	Legacy Skeleton formats have been designed up to a decent prototype
	level for now.  Also I am converting most of the project to C++ as I
	have found out that Doom 3 was written in C++ and I feel that it will
	not have to much of a speed impact to matter.  Also I feel that the
	project should not be called Legacy 3D, but simply the Legacy Engine
	instead.
	
3/25/2007 - Work on the project continues.  I'm am moving away from mesh
	rasterization for a while (though certainly it still needs some
	improvements).  Recently I added very basic collision detetion between
	entities.  I have also decided on 3D World Studio for the editor and
	the next big projects will be map rasterization/and map collision.
	I feel a bigger inclination to develop a portal system for the maps 
	(as opposed to bsp), though it will still be quite a while until the
	initial map rendering will being.
	
5/27/2007 - Most of the work I've done recently has been done on the file
	system, of which I'm pretty satisfied with what I've done.  The file
	system is lf_sys2.dll and it features a mounting system with a hash
	index for quick access to specific files.  The new file system has no
	dependencies on the OS file system, and in fact absolutely no OS
	file system functions should be called while using the Legacy file
	system.  I've also changed the lpk format (though the old format is
	still readable) and designed a new archiver call PaKager.  The new
	file system is much more robust and is ANSI and UNICODE compatible,
	also uncompressed archived files can be streamed in the same way
	that a file on the disk can be streamed (that is to say an uncompressed
	archived file does not need to be opened into memory, so large
	audio and movie files can be streamed out of an archive).  Compressed
	files are still opened into memory.
	
6/28/2007 - It's been well over a year since I've started this project
	and I am happy to anounce that I have finally begun developing
	actually engine mechanics.  No longer are all the entities and
	graphics processed using the CLVTest class, there is now a CLWorld
	class which contains all the world information including the map
	(which includes rasterization data and geometry) and all the
	entities within the world.  The CLWorld class then processes
	the entities and rasterizes them.  The game engine is now looking
	like an actually game, and not just an application that renders
	some meshes, and pictures.  I am also working on cleaning up
	the code and insuring that it is easier to read.  As of now I
	need to work on game mechanics/physics and I want to redo the
	console as well.  That said, things are looking pretty good and
	it has only been the past month or so that I've made this
	progress.
	
6/4/2008 - Quite a bit of progress has been made, but also quite
	a bit of work has been thrown out, because it just wasn't very
	well designed.  Right now the game is looking alright, I've created
	quite a few script readers, and worked on the physics engine
	quite a bit.  Further I redesigned the entire architecture so
	that the server, client, and physics engines are all separate
	classes that communicate with each other.  I've also added pixel
	and vertex shader support.  I'm mostly interested in developing
	the physics engine right now, and as of now the engine supports
	three engines:  The Legacy physics engine developed by Beem. 
	The Newton Game Dynamics physics engine develop by Newton Game
	Dynamics.  And the PhysX physics engine developed by NVIDIA
	(originally AGEIA).  Honestly I think the PhysX one is the most
	promising, but I want to work with the other two for experience
	sakes.  I'm pretty happy with the progress I've made even though
	I've had to throw out a lot of code.  The truth is that as the
	code stands today, almost nothing of the original 2006 code 
	exists anymore.  I still have some remnant files that I am refering
	to from time to time, but I imagine in a few days or weeks they
	will be completely thrown out (the le_* files).  There are a few
	changes that I want to make to lf_sys2, and once I do that I will
	consider lf_sys2 complete, and move the project out of the solution,
	and simply link to it through the 3rdParty folder.
	
6/10/2008 - So I redid a lot of work on the Legacy Mesh code, I didn't
	change the format at all, but the code is now more streamlined, and
	it isn't such a mess when going from the MilkShape exporter, to
	LMEdit, to the actual game.  I'm now using inherrited classes, that
	have a few required overrides, (such as the Load and Save methods)
	which mean that those methods must be overriden for specific instances
	and in that way we overall don't have such a mess (with things like
	#ifdef LM_USE_LF2) etc.  The mesh node class still needs some work,
	but I'll soon be able to focus on server side development, and I can
	ignore the client side for a while.
	
	Also I've made generally readability improvements to the code, that
	means better organization, and more remarks, etc.
	
6/25/2008 - I've finalized the mesh code for now, and the animation
	system looks pretty smooth, considering I don't have that many
	animations.  I don't think I'll have to work on the mesh system
	for a while, and it's probably fine for a demo.  One the server
	side of things, PhysX is looking the best for the physics engine, so
	I'll focus most of my development on that, and probably drop Newton
	altogether, I may work some more on the in house physics engine, but
	I don't see any reason to put much focus into it.
	
5/11/2009 - Legacy is too much of a mess to work with anymore. The code
	is a rediculous combination of C and C++, there are a lot of
	inconsitencies throughout. Early code shows a lot of bad programming
	practice, the files I'm using for the game are inconsistent. The
	project, unfortunately, has to be scrapped. If I want to complete a
	demo before I graduate, I need to focus on something simple. I tried
	to make this way more complex than it needed to be for a simple demo.
	There is way too much code. I'm going to have to start a completely
	new project. The good news is that the code I created that is
	reusable will be just that, reused. The file system is solid so I
	will be able to use that just fine. The mesh code and animation
	skeleton code is pretty good, I need to fix the XML loading stuff,
	but other than that it is usable. The map format is usable, though
	I still very much need to fix the physics system.
	
	The new engine will be called Emergence, since the game will be
	"The Legacy of Heaven & Hell: Emergence, instead of "LH&H: A Lantern
	in the Dark".
</pre>

### Original Credtis.txt
<pre>
"Legacy 3D" Copyright (c) 2006, Blaine Myers

"zlib" Copyright (c) 1995-2005 Jean-loup Gailly and Mark Adler
version 1.2.3, July 18th, 2005

Ogg Vorbis Copyright (c) 2006, Xiph.Org Foundation
"libvorbis" version 1.1.2 "libogg" version 1.1.3

All other trademarks and trade names are the property of their 
respective owners.

This software is provided by the copyright holders and contributors "as is" and any express or implied warranties, including, but not limited to, the implied warranties of merchantability and fitness for a particular purpose are disclaimed. In no event shall the foundation or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.
</pre>

