##----------------------------------------------------------------
## Include files.
##----------------------------------------------------------------
!include "..\Projects\version.txt"
!include "parsed_proj.txt"
!include "sign_config.txt"

##----------------------------------------------------------------
## Project directoires.
##----------------------------------------------------------------
!if "$(CONF)" == "DEBUG"
OutDir = ..\Output\Evo\Files\Debug
!else
OutDir = ..\Output\Evo\Files\Release
!endif
SignDir = $(OutDir)\Sign\Flash
SrcDir = ..\Source
ObjDir = $(OutDir)\Obj
ProjDir = ..\..\..\evo_http_lib

Objects = $(SOURCES:.c=.o)
!if "$(CONF)" == "DEBUG"
Objects = $(Objects:..\Source=..\Output\Evo\Files\Debug\Obj)
!else
Objects = $(Objects:..\Source=..\Output\Evo\Files\Release\Obj)
!endif

##----------------------------------------------------------------
## Compiler options.
##----------------------------------------------------------------
!if "$(CONF)" == "DEBUG"
COptions = -p -v -g -c -shlabs -armcc,--C99
!else
COptions = -p -v -c -shlabs -armcc,--C99
!endif

##----------------------------------------------------------------
## Linker options.
##----------------------------------------------------------------
!if "$(CONF)" == "DEBUG"
LOptions = -p -v -g -map -shlpic
!else
LOptions = -p -v -shlpic
!endif

##----------------------------------------------------------------
## Build configuration.
##----------------------------------------------------------------
build: $(SignDir)\$(OUTPUTFILE).lib.p7s

##----------------------------------------------------------------
## Sign shared library.
##----------------------------------------------------------------
$(SignDir)\$(OUTPUTFILE).lib.p7s : BinaryConfiguration
# Empty remote directory.
	del $(RemoteDir)\test2\* /Q
# Copy output file.
	copy $(OutDir)\$(OUTPUTFILE).lib $(RemoteDir)\test2\$(OUTPUTFILE).lib
# Generate FST file.
	psexec $(RemoteHost) -u $(RemoteUser) -p $(RemotePwd) $(DirOnRemote)\$(SignTool) test2/$(OUTPUTFILE).lib
# Generate file signature.
	psexec $(RemoteHost) -u $(RemoteUser) -p $(RemotePwd) $(FileSignatureTool)  $(DirOnRemote)\test2\$(OUTPUTFILE).lib.fst
# Download file signature.
	copy $(RemoteDir)\test2\$(OUTPUTFILE).lib.p7s $(SignDir)\$(OUTPUTFILE).lib.p7s

##----------------------------------------------------------------
## Output configuration.
##----------------------------------------------------------------
BinaryConfiguration : $(OutDir)\$(OUTPUTFILE).lib
# Set library version.
	$(EVOSDK)\bin\vrxhdr -v $(VER) $**

##----------------------------------------------------------------
## Create shared library out of httplib archive.
##----------------------------------------------------------------
$(OutDir)\$(OUTPUTFILE).lib : $(OutDir)\$(OUTPUTFILE).a ..\Projects\$(OUTPUTFILE).lid
# Copy lid file to output dir.
	copy ..\Projects\$(OUTPUTFILE).lid $(OutDir)\$(OUTPUTFILE).lid
# Change directory to output dir (vrxlib tool requirement).
	pushd $(OutDir) & $(EVOSDK)\bin\vrxlib -p $(OUTPUTFILE) & popd
# Call vrxlib.
# Remove copied *.lid file.
	del $(OutDir)\$(OUTPUTFILE).lid

##----------------------------------------------------------------
## Link.
##----------------------------------------------------------------
$(OutDir)\$(OUTPUTFILE).a: $(Objects) $(LIBS)
	$(EVOSDK)\bin\evocc $(LOptions) $** -o $@

##----------------------------------------------------------------
## Compile.
##----------------------------------------------------------------
{$(SrcDir)}.c{$(ObjDir)}.o:
    echo Compiling '$(@B).c':
    $(EVOSDK)\bin\evocc $(COptions) $(INCLUDES) $(DEFINES) $(SrcDir)\$(@B).c -o $@

##----------------------------------------------------------------
## Clean configuration.
##----------------------------------------------------------------
clean:
    del $(OutDir)\*.* /Q
    del $(SignDir)\*.* /Q
    del $(ObjDir)\*.* /Q

##----------------------------------------------------------------
## Install configuration.
##----------------------------------------------------------------
install:
    echo Not implemented yet.
