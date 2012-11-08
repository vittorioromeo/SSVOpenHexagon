##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=SSVOpenHexagon
ConfigurationName      :=Debug
WorkspacePath          := "D:\Vee\Software\WIP\CL\WorkspaceOH\OH"
ProjectPath            := "D:\Vee\Software\WIP\CL\WorkspaceOH\OH\SSVOpenHexagon"
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Vittorio
Date                   :=11/08/12
CodeLitePath           :="c:\Program Files (x86)\CodeLite"
LinkerName             :=g++
SharedObjectLinkerName :=g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.o.i
DebugSwitch            :=-gstab
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName).exe
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E 
ObjectsFileList        :="D:\Vee\Software\WIP\CL\WorkspaceOH\OH\SSVOpenHexagon\SSVOpenHexagon.txt"
PCHCompileFlags        :=
MakeDirCommand         :=makedir
RcCmpOptions           := 
RcCompilerName         :=windres
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch)../SSVEntitySystem $(IncludeSwitch)../SSVStart $(IncludeSwitch)D:/Vee/Software/WIP/SFMLMinGW/include 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)SSVEntitySystem $(LibrarySwitch)SSVStart $(LibrarySwitch)sfml-window-d $(LibrarySwitch)sfml-graphics-d $(LibrarySwitch)sfml-system-d $(LibrarySwitch)sfml-audio-d 
ArLibs                 :=  "SSVEntitySystem" "SSVStart" "sfml-window-d" "sfml-graphics-d" "sfml-system-d" "sfml-audio-d" 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)../SSVStart/Debug $(LibraryPathSwitch)../SSVEntitySystem/Debug $(LibraryPathSwitch)D:/Vee/Software/WIP/SFMLMinGW/lib 

##
## Common variables
## AR, CXX, CC, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := ar rcus
CXX      := g++
CC       := gcc
CXXFLAGS :=  -pedantic -Wall -O0 -g3 -Wextra -c -std=c++11 $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)


##
## User defined environment variables
##
CodeLiteDir:=c:\Program Files (x86)\CodeLite
WXWIN:=C:\wxWidgets
UNIT_TEST_PP_SRC_DIR:=C:\UnitTest++-1.3
WXCFG:=gcc_dll\mswu
Objects=$(IntermediateDirectory)/main$(ObjectSuffix) $(IntermediateDirectory)/CPlayer$(ObjectSuffix) $(IntermediateDirectory)/CWall$(ObjectSuffix) $(IntermediateDirectory)/HexagonGame$(ObjectSuffix) $(IntermediateDirectory)/PatternManager$(ObjectSuffix) $(IntermediateDirectory)/Utils$(ObjectSuffix) $(IntermediateDirectory)/LevelSettings$(ObjectSuffix) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects) > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

$(IntermediateDirectory)/.d:
	@$(MakeDirCommand) "./Debug"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/main$(ObjectSuffix): main.cpp $(IntermediateDirectory)/main$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/WIP/CL/WorkspaceOH/OH/SSVOpenHexagon/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/main$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/main$(DependSuffix): main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/main$(ObjectSuffix) -MF$(IntermediateDirectory)/main$(DependSuffix) -MM "D:/Vee/Software/WIP/CL/WorkspaceOH/OH/SSVOpenHexagon/main.cpp"

$(IntermediateDirectory)/main$(PreprocessSuffix): main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/main$(PreprocessSuffix) "D:/Vee/Software/WIP/CL/WorkspaceOH/OH/SSVOpenHexagon/main.cpp"

$(IntermediateDirectory)/CPlayer$(ObjectSuffix): CPlayer.cpp $(IntermediateDirectory)/CPlayer$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/WIP/CL/WorkspaceOH/OH/SSVOpenHexagon/CPlayer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/CPlayer$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/CPlayer$(DependSuffix): CPlayer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/CPlayer$(ObjectSuffix) -MF$(IntermediateDirectory)/CPlayer$(DependSuffix) -MM "D:/Vee/Software/WIP/CL/WorkspaceOH/OH/SSVOpenHexagon/CPlayer.cpp"

$(IntermediateDirectory)/CPlayer$(PreprocessSuffix): CPlayer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/CPlayer$(PreprocessSuffix) "D:/Vee/Software/WIP/CL/WorkspaceOH/OH/SSVOpenHexagon/CPlayer.cpp"

$(IntermediateDirectory)/CWall$(ObjectSuffix): CWall.cpp $(IntermediateDirectory)/CWall$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/WIP/CL/WorkspaceOH/OH/SSVOpenHexagon/CWall.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/CWall$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/CWall$(DependSuffix): CWall.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/CWall$(ObjectSuffix) -MF$(IntermediateDirectory)/CWall$(DependSuffix) -MM "D:/Vee/Software/WIP/CL/WorkspaceOH/OH/SSVOpenHexagon/CWall.cpp"

$(IntermediateDirectory)/CWall$(PreprocessSuffix): CWall.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/CWall$(PreprocessSuffix) "D:/Vee/Software/WIP/CL/WorkspaceOH/OH/SSVOpenHexagon/CWall.cpp"

$(IntermediateDirectory)/HexagonGame$(ObjectSuffix): HexagonGame.cpp $(IntermediateDirectory)/HexagonGame$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/WIP/CL/WorkspaceOH/OH/SSVOpenHexagon/HexagonGame.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/HexagonGame$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/HexagonGame$(DependSuffix): HexagonGame.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/HexagonGame$(ObjectSuffix) -MF$(IntermediateDirectory)/HexagonGame$(DependSuffix) -MM "D:/Vee/Software/WIP/CL/WorkspaceOH/OH/SSVOpenHexagon/HexagonGame.cpp"

$(IntermediateDirectory)/HexagonGame$(PreprocessSuffix): HexagonGame.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/HexagonGame$(PreprocessSuffix) "D:/Vee/Software/WIP/CL/WorkspaceOH/OH/SSVOpenHexagon/HexagonGame.cpp"

$(IntermediateDirectory)/PatternManager$(ObjectSuffix): PatternManager.cpp $(IntermediateDirectory)/PatternManager$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/WIP/CL/WorkspaceOH/OH/SSVOpenHexagon/PatternManager.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/PatternManager$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/PatternManager$(DependSuffix): PatternManager.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/PatternManager$(ObjectSuffix) -MF$(IntermediateDirectory)/PatternManager$(DependSuffix) -MM "D:/Vee/Software/WIP/CL/WorkspaceOH/OH/SSVOpenHexagon/PatternManager.cpp"

$(IntermediateDirectory)/PatternManager$(PreprocessSuffix): PatternManager.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/PatternManager$(PreprocessSuffix) "D:/Vee/Software/WIP/CL/WorkspaceOH/OH/SSVOpenHexagon/PatternManager.cpp"

$(IntermediateDirectory)/Utils$(ObjectSuffix): Utils.cpp $(IntermediateDirectory)/Utils$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/WIP/CL/WorkspaceOH/OH/SSVOpenHexagon/Utils.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Utils$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Utils$(DependSuffix): Utils.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Utils$(ObjectSuffix) -MF$(IntermediateDirectory)/Utils$(DependSuffix) -MM "D:/Vee/Software/WIP/CL/WorkspaceOH/OH/SSVOpenHexagon/Utils.cpp"

$(IntermediateDirectory)/Utils$(PreprocessSuffix): Utils.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Utils$(PreprocessSuffix) "D:/Vee/Software/WIP/CL/WorkspaceOH/OH/SSVOpenHexagon/Utils.cpp"

$(IntermediateDirectory)/LevelSettings$(ObjectSuffix): LevelSettings.cpp $(IntermediateDirectory)/LevelSettings$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/WIP/CL/WorkspaceOH/OH/SSVOpenHexagon/LevelSettings.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/LevelSettings$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/LevelSettings$(DependSuffix): LevelSettings.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/LevelSettings$(ObjectSuffix) -MF$(IntermediateDirectory)/LevelSettings$(DependSuffix) -MM "D:/Vee/Software/WIP/CL/WorkspaceOH/OH/SSVOpenHexagon/LevelSettings.cpp"

$(IntermediateDirectory)/LevelSettings$(PreprocessSuffix): LevelSettings.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/LevelSettings$(PreprocessSuffix) "D:/Vee/Software/WIP/CL/WorkspaceOH/OH/SSVOpenHexagon/LevelSettings.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) $(IntermediateDirectory)/main$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/main$(DependSuffix)
	$(RM) $(IntermediateDirectory)/main$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/CPlayer$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/CPlayer$(DependSuffix)
	$(RM) $(IntermediateDirectory)/CPlayer$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/CWall$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/CWall$(DependSuffix)
	$(RM) $(IntermediateDirectory)/CWall$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/HexagonGame$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/HexagonGame$(DependSuffix)
	$(RM) $(IntermediateDirectory)/HexagonGame$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/PatternManager$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/PatternManager$(DependSuffix)
	$(RM) $(IntermediateDirectory)/PatternManager$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Utils$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Utils$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Utils$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/LevelSettings$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/LevelSettings$(DependSuffix)
	$(RM) $(IntermediateDirectory)/LevelSettings$(PreprocessSuffix)
	$(RM) $(OutputFile)
	$(RM) $(OutputFile).exe
	$(RM) "D:\Vee\Software\WIP\CL\WorkspaceOH\OH\.build-debug\SSVOpenHexagon"


