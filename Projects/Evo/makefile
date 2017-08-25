##----------------------------------------------------------------
## Check makefile requirements.
##----------------------------------------------------------------
!ifdef ProjDir
!ifdef DEBUG

##----------------------------------------------------------------
## Project directoires.
##----------------------------------------------------------------
!if $(DEBUG) == 0
OutDir = $(ProjDir)\Output\Evo\Files\Release
SignDir = $(ProjDir)\Output\Evo\Files\Release\Sign\Flash
!else
OutDir = $(ProjDir)\Output\Evo\Files\Debug
SignDir = $(ProjDir)\Output\Evo\Files\Debug\Sign\Flash
!endif
SrcDir = $(ProjDir)\Source
VCSLibDir = C:\Users\specak\Documents\gitlab\evo_vcs_lib

##----------------------------------------------------------------
## File signature variables.
##----------------------------------------------------------------
RemoteDir = \\192.168.65.17\dpajak\sign
DirOnRemote = D:\shared\dpajak\sign
FileSignatureTool = "C:\Program Files\Verifone\fst\filesignature.exe"
RemoteHost = \\onir
RemoteUser = mp
RemotePwd = mp
SignTool = signfile_evo.cmd
##----------------------------------------------------------------
## All required include files.
##----------------------------------------------------------------
SDKIncludes = $(EVOSDK)\Include
#VMACIncludes = $(EVOVMAC)\Include
SelfIncludes = $(ProjDir)\Include
VCSLibIncludes = $(VCSLibDir)\Include

##----------------------------------------------------------------
## All includes packed as compiler parameters.
##----------------------------------------------------------------
Includes = -I$(SDKIncludes) -I$(SelfIncludes) -I$(VCSLibIncludes)

##----------------------------------------------------------------
## Output name.
##----------------------------------------------------------------
LibNameWork = httplib
LibNameOut = http

##----------------------------------------------------------------
## Compiler options.
##----------------------------------------------------------------
!if $(DEBUG) == 0
COptions = -p -v -shlpic -c
!else
COptions = -p -v -g -shlpic -c
!endif

##----------------------------------------------------------------
## Linker options.
##----------------------------------------------------------------
!if $(DEBUG) == 0
LOptions = -p -v -shlpic
!else
LOptions = -p -v -shlpic -g
!endif

##----------------------------------------------------------------
## Linkable objects.
##----------------------------------------------------------------
!if $(DEBUG) == 0
VCSLib = $(VCSLibDir)\Output\Evo\Files\Release\vcslib.a
VMACLib = $(EVOVMAC)\Output\RV\Lib\Files\Release\vmac.o
!else
VCSLib = $(VCSLibDir)\Output\Evo\Files\Debug\vcslib.a
VMACLib = $(EVOVMAC)\Output\RV\Lib\Files\Debug\vmac.o
!endif

##----------------------------------------------------------------
## Build configuration.
##----------------------------------------------------------------
pseudoOut : $(SignDir)\$(LibNameOut).lib.p7s

##----------------------------------------------------------------
## Sign shared library.
##----------------------------------------------------------------
$(SignDir)\$(LibNameOut).lib.p7s : $(OutDir)\$(LibNameOut).lib
# Empty remote directory.
	del $(RemoteDir)\test2\* /Q
# Copy output file.
	copy $(OutDir)\$(LibNameOut).lib $(RemoteDir)\test2\$(LibNameOut).lib
# Generate FST file.
	psexec $(RemoteHost) -u $(RemoteUser) -p $(RemotePwd) $(DirOnRemote)\$(SignTool) test2/$(LibNameOut).lib
# Generate file signature.
	psexec $(RemoteHost) -u $(RemoteUser) -p $(RemotePwd) $(FileSignatureTool)  $(DirOnRemote)\test2\$(LibNameOut).lib.fst
# Download file signature.
	copy $(RemoteDir)\test2\$(LibNameOut).lib.p7s $(SignDir)\$(LibNameOut).lib.p7s

##----------------------------------------------------------------
## Create shared library out of httplib archive.
##----------------------------------------------------------------
$(OutDir)\$(LibNameOut).lib : $(OutDir)\$(LibNameWork).a $(ProjDir)\Projects\$(LibNameWork).lid
# Copy lid file to output dir.
	copy $(ProjDir)\Projects\$(LibNameWork).lid $(OutDir)\$(LibNameWork).lid
# Change directory to output dir (vrxlib tool requirement).
	cd $(OutDir)
# Call vrxlib.
	$(EVOSDK)\bin\vrxlib -p $(LibNameWork)
# Remove copied *.lid file.
	del $(OutDir)\$(LibNameWork).lid /Q
# Rename library file.
	move $(OutDir)\$(LibNameWork).lib $(OutDir)\$(LibNameOut).lib

##----------------------------------------------------------------
## Link.
##----------------------------------------------------------------
$(OutDir)\$(LibNameWork).a : $(OutDir)\$(LibNameWork).o $(VCSLib) $(VMACLib)
	$(EVOSDK)\bin\vrxcc $(LOptions) $** -o $@

##----------------------------------------------------------------
## Compile.
##----------------------------------------------------------------
$(OutDir)\$(LibNameWork).o : $(SrcDir)\$(LibNameWork).c
	$(EVOSDK)\bin\vrxcc $(COptions) $(Includes) $** -o $@

##----------------------------------------------------------------
## Clean configuration.
##----------------------------------------------------------------
clean:
	del $(OutDir)\*.a /Q
	del $(OutDir)\*.o /Q
	del $(OutDir)\*.lib /Q
	del $(OutDir)\*.axf /Q
	del $(OutDir)\*.map /Q
	del $(OutDir)\*.err /Q

##----------------------------------------------------------------
## If DEBUG was not defined.
##----------------------------------------------------------------
!else

!error You forgot to define 'DEBUG' macro containing project's base directory! 

!endif

##----------------------------------------------------------------
## If ProjDir was not defined.
##----------------------------------------------------------------
!else

!error You forgot to define 'ProjDir' macro containing project's base directory! 

!endif