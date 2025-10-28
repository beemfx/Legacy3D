# Microsoft Developer Studio Project File - Name="legacy3d" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=legacy3d - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "legacy3d.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "legacy3d.mak" CFG="legacy3d - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "legacy3d - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "legacy3d - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "legacy3d - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\common" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "LEGACY3D_GAME" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "legacy3d - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\common" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "LEGACY3D_GAME" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /profile /debug /machine:I386 /out:"Debug/legacy3d_d.exe"
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "legacy3d - Win32 Release"
# Name "legacy3d - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "ASM Source Files"

# PROP Default_Filter "a, asm"
# Begin Source File

SOURCE=.\lm_math_asm.a

!IF  "$(CFG)" == "legacy3d - Win32 Release"

# Begin Custom Build - Assembling $(InputPath)...
TargetDir=.\Release
InputPath=.\lm_math_asm.a
InputName=lm_math_asm

"$(TargetDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw.exe -f win32 $(InputPath) -o $(TargetDir)\$(InputName).obj -X vc

# End Custom Build

!ELSEIF  "$(CFG)" == "legacy3d - Win32 Debug"

# Begin Custom Build - Assembling $(InputPath)...
TargetDir=.\Debug
InputPath=.\lm_math_asm.a
InputName=lm_math_asm

"$(TargetDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw.exe -f win32 $(InputPath) -o $(TargetDir)\$(InputName).obj -X vc

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=..\common\common.c
# End Source File
# Begin Source File

SOURCE=.\legacy3d.rc
# End Source File
# Begin Source File

SOURCE=.\lg_cmd.c
# End Source File
# Begin Source File

SOURCE=.\lg_err.c
# End Source File
# Begin Source File

SOURCE=.\lg_init.c
# End Source File
# Begin Source File

SOURCE=.\lg_sys.c
# End Source File
# Begin Source File

SOURCE=.\lm_d3d.cpp
# End Source File
# Begin Source File

SOURCE=.\lm_math.c
# End Source File
# Begin Source File

SOURCE=.\lm_skel.cpp
# End Source File
# Begin Source File

SOURCE=.\lm_sys.cpp
# End Source File
# Begin Source File

SOURCE=.\ls_init.c
# End Source File
# Begin Source File

SOURCE=.\ls_load.c
# End Source File
# Begin Source File

SOURCE=.\ls_ogg.c
# End Source File
# Begin Source File

SOURCE=.\ls_wav.c
# End Source File
# Begin Source File

SOURCE=.\lv_con.c
# End Source File
# Begin Source File

SOURCE=.\lv_font2.c
# End Source File
# Begin Source File

SOURCE=.\lv_img2d.c
# End Source File
# Begin Source File

SOURCE=.\lv_init.c
# End Source File
# Begin Source File

SOURCE=.\lv_reset.c
# End Source File
# Begin Source File

SOURCE=.\lv_test.cpp
# End Source File
# Begin Source File

SOURCE=.\lv_tex.c
# End Source File
# Begin Source File

SOURCE=.\lv_tex_img_lib.c
# End Source File
# Begin Source File

SOURCE=.\win_sys.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\common\common.h
# End Source File
# Begin Source File

SOURCE=.\lg_cmd.h
# End Source File
# Begin Source File

SOURCE=.\lg_err.h
# End Source File
# Begin Source File

SOURCE=.\lg_init.h
# End Source File
# Begin Source File

SOURCE=.\lg_sys.h
# End Source File
# Begin Source File

SOURCE=.\lm_d3d.h
# End Source File
# Begin Source File

SOURCE=.\lm_math.h
# End Source File
# Begin Source File

SOURCE=.\lm_sys.h
# End Source File
# Begin Source File

SOURCE=.\ls_init.h
# End Source File
# Begin Source File

SOURCE=.\ls_load.h
# End Source File
# Begin Source File

SOURCE=.\ls_ogg.h
# End Source File
# Begin Source File

SOURCE=.\ls_wav.h
# End Source File
# Begin Source File

SOURCE=.\lv_con.h
# End Source File
# Begin Source File

SOURCE=.\lv_font2.h
# End Source File
# Begin Source File

SOURCE=.\lv_img2d.h
# End Source File
# Begin Source File

SOURCE=.\lv_init.h
# End Source File
# Begin Source File

SOURCE=.\lv_reset.h
# End Source File
# Begin Source File

SOURCE=.\lv_test.h
# End Source File
# Begin Source File

SOURCE=.\lv_tex.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\appicon.ico
# End Source File
# Begin Source File

SOURCE=.\l3d.ico
# End Source File
# Begin Source File

SOURCE=.\l3dico.ico
# End Source File
# End Group
# Begin Group "DX Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\..\SDK\DX(June06)\Include\d3d9.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\SDK\DX(June06)\Include\d3d9caps.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\SDK\DX(June06)\Include\d3d9types.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\SDK\DX(June06)\Include\d3dx9.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\SDK\DX(June06)\Include\d3dx9anim.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\SDK\DX(June06)\Include\d3dx9core.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\SDK\DX(June06)\Include\d3dx9effect.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\SDK\DX(June06)\Include\d3dx9math.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\SDK\DX(June06)\Include\d3dx9mesh.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\SDK\DX(June06)\Include\d3dx9shader.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\SDK\DX(June06)\Include\d3dx9shape.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\SDK\DX(June06)\Include\d3dx9tex.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\SDK\DX(June06)\Include\d3dx9xof.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\SDK\DX(June06)\Include\dinput.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\SDK\DX(June06)\Include\dsound.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\SDK\DX(June06)\Include\dxerr9.h"
# End Source File
# End Group
# Begin Source File

SOURCE=.\DevelopMentLog.txt
# End Source File
# End Target
# End Project
