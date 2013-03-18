##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Release
ProjectName            :=SSVOpenHexagon
ConfigurationName      :=Release
WorkspacePath          := "D:\Vee\Software\GitHub\OHWorkspace"
ProjectPath            := "D:\Vee\Software\GitHub\OHWorkspace\SSVOpenHexagon"
IntermediateDirectory  :=./_INTERMEDIATE
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=vittorio.romeo
Date                   :=18/03/2013
CodeLitePath           :="C:\Program Files (x86)\CodeLite"
LinkerName             :=g++
SharedObjectLinkerName :=g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.o.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=./_RELEASE/$(ProjectName).exe
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="SSVOpenHexagon.txt"
PCHCompileFlags        :=
MakeDirCommand         :=makedir
RcCmpOptions           := 
RcCompilerName         :=windres
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch)../SSVEntitySystem $(IncludeSwitch)../SSVStart $(IncludeSwitch)../SSVLuaWrapper $(IncludeSwitch)../SSVMenuSystem $(IncludeSwitch)D:/Vee/Software/GitHub/OHWorkspace/SFML/include $(IncludeSwitch)D:/Vee/Software/GitHub/OHWorkspace/jsoncpp/include $(IncludeSwitch)C:/lua/include $(IncludeSwitch)D:/Vee/Software/Repos/sparsehash/build/built/include 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)SSVEntitySystem $(LibrarySwitch)SSVStart $(LibrarySwitch)SSVLuaWrapper $(LibrarySwitch)SSVMenuSystem $(LibrarySwitch)sfml-window $(LibrarySwitch)sfml-graphics $(LibrarySwitch)sfml-system $(LibrarySwitch)sfml-audio $(LibrarySwitch)sfml-network $(LibrarySwitch)json_mingw_libmt $(LibrarySwitch)lua5.1 $(LibrarySwitch)lua51 
ArLibs                 :=  "SSVEntitySystem" "SSVStart" "SSVLuaWrapper" "SSVMenuSystem" "sfml-window" "sfml-graphics" "sfml-system" "sfml-audio" "sfml-network" "json_mingw_libmt" "lua5.1" "lua51" 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)../SSVStart/_RELEASE $(LibraryPathSwitch)../SSVEntitySystem/_RELEASE $(LibraryPathSwitch)../SSVLuaWrapper/_RELEASE $(LibraryPathSwitch)../SSVSCollision/_RELEASE $(LibraryPathSwitch)../SSVMenuSystem/_RELEASE $(LibraryPathSwitch)D:/Vee/Software/GitHub/OHWorkspace/SFML/build2/lib $(LibraryPathSwitch)D:/Vee/Software/GitHub/OHWorkspace/jsoncpp/libs/mingw $(LibraryPathSwitch)c:/lua 

##
## Common variables
## AR, CXX, CC, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := ar rcus
CXX      := g++
CC       := gcc
CXXFLAGS :=  -W -s -pedantic -O3 -Wextra -std=c++11 -Wall $(Preprocessors)
CFLAGS   :=  -O2 -Wall $(Preprocessors)


##
## User defined environment variables
##
CodeLiteDir:=C:\Program Files (x86)\CodeLite
UNIT_TEST_PP_SRC_DIR:=C:\UnitTest++-1.3
WXWIN:=C:\wxWidgets-2.9.4
WXCFG:=gcc_dll\mswu
Objects0=$(IntermediateDirectory)/main$(ObjectSuffix) $(IntermediateDirectory)/HexagonGame$(ObjectSuffix) $(IntermediateDirectory)/MenuGame$(ObjectSuffix) $(IntermediateDirectory)/HGScripting$(ObjectSuffix) $(IntermediateDirectory)/HGUpdate$(ObjectSuffix) $(IntermediateDirectory)/HGProperties$(ObjectSuffix) $(IntermediateDirectory)/HGGraphics$(ObjectSuffix) $(IntermediateDirectory)/Components_CPlayer$(ObjectSuffix) $(IntermediateDirectory)/Components_CWall$(ObjectSuffix) $(IntermediateDirectory)/Global_Assets$(ObjectSuffix) \
	$(IntermediateDirectory)/Global_Config$(ObjectSuffix) $(IntermediateDirectory)/Global_Factory$(ObjectSuffix) $(IntermediateDirectory)/Data_StyleData$(ObjectSuffix) $(IntermediateDirectory)/Data_LevelData$(ObjectSuffix) $(IntermediateDirectory)/Data_ProfileData$(ObjectSuffix) $(IntermediateDirectory)/Data_EventData$(ObjectSuffix) $(IntermediateDirectory)/Data_PackData$(ObjectSuffix) $(IntermediateDirectory)/Data_MusicData$(ObjectSuffix) $(IntermediateDirectory)/Utils_Utils$(ObjectSuffix) $(IntermediateDirectory)/Utils_MD5$(ObjectSuffix) \
	$(IntermediateDirectory)/Utils_FPSWatcher$(ObjectSuffix) $(IntermediateDirectory)/Utils_Base64$(ObjectSuffix) $(IntermediateDirectory)/Online_Online$(ObjectSuffix) $(IntermediateDirectory)/Compatibility_Compatibility$(ObjectSuffix) 

Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) $(Objects) $(LibPath) $(Libs) $(LinkOptions)

$(IntermediateDirectory)/.d:
	@$(MakeDirCommand) "./_INTERMEDIATE"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/main$(ObjectSuffix): main.cpp $(IntermediateDirectory)/main$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/main$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/main$(DependSuffix): main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/main$(ObjectSuffix) -MF$(IntermediateDirectory)/main$(DependSuffix) -MM "main.cpp"

$(IntermediateDirectory)/main$(PreprocessSuffix): main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/main$(PreprocessSuffix) "main.cpp"

$(IntermediateDirectory)/HexagonGame$(ObjectSuffix): HexagonGame.cpp $(IntermediateDirectory)/HexagonGame$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/HexagonGame.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/HexagonGame$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/HexagonGame$(DependSuffix): HexagonGame.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/HexagonGame$(ObjectSuffix) -MF$(IntermediateDirectory)/HexagonGame$(DependSuffix) -MM "HexagonGame.cpp"

$(IntermediateDirectory)/HexagonGame$(PreprocessSuffix): HexagonGame.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/HexagonGame$(PreprocessSuffix) "HexagonGame.cpp"

$(IntermediateDirectory)/MenuGame$(ObjectSuffix): MenuGame.cpp $(IntermediateDirectory)/MenuGame$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/MenuGame.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/MenuGame$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/MenuGame$(DependSuffix): MenuGame.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/MenuGame$(ObjectSuffix) -MF$(IntermediateDirectory)/MenuGame$(DependSuffix) -MM "MenuGame.cpp"

$(IntermediateDirectory)/MenuGame$(PreprocessSuffix): MenuGame.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/MenuGame$(PreprocessSuffix) "MenuGame.cpp"

$(IntermediateDirectory)/HGScripting$(ObjectSuffix): HGScripting.cpp $(IntermediateDirectory)/HGScripting$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/HGScripting.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/HGScripting$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/HGScripting$(DependSuffix): HGScripting.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/HGScripting$(ObjectSuffix) -MF$(IntermediateDirectory)/HGScripting$(DependSuffix) -MM "HGScripting.cpp"

$(IntermediateDirectory)/HGScripting$(PreprocessSuffix): HGScripting.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/HGScripting$(PreprocessSuffix) "HGScripting.cpp"

$(IntermediateDirectory)/HGUpdate$(ObjectSuffix): HGUpdate.cpp $(IntermediateDirectory)/HGUpdate$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/HGUpdate.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/HGUpdate$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/HGUpdate$(DependSuffix): HGUpdate.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/HGUpdate$(ObjectSuffix) -MF$(IntermediateDirectory)/HGUpdate$(DependSuffix) -MM "HGUpdate.cpp"

$(IntermediateDirectory)/HGUpdate$(PreprocessSuffix): HGUpdate.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/HGUpdate$(PreprocessSuffix) "HGUpdate.cpp"

$(IntermediateDirectory)/HGProperties$(ObjectSuffix): HGProperties.cpp $(IntermediateDirectory)/HGProperties$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/HGProperties.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/HGProperties$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/HGProperties$(DependSuffix): HGProperties.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/HGProperties$(ObjectSuffix) -MF$(IntermediateDirectory)/HGProperties$(DependSuffix) -MM "HGProperties.cpp"

$(IntermediateDirectory)/HGProperties$(PreprocessSuffix): HGProperties.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/HGProperties$(PreprocessSuffix) "HGProperties.cpp"

$(IntermediateDirectory)/HGGraphics$(ObjectSuffix): HGGraphics.cpp $(IntermediateDirectory)/HGGraphics$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/HGGraphics.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/HGGraphics$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/HGGraphics$(DependSuffix): HGGraphics.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/HGGraphics$(ObjectSuffix) -MF$(IntermediateDirectory)/HGGraphics$(DependSuffix) -MM "HGGraphics.cpp"

$(IntermediateDirectory)/HGGraphics$(PreprocessSuffix): HGGraphics.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/HGGraphics$(PreprocessSuffix) "HGGraphics.cpp"

$(IntermediateDirectory)/Components_CPlayer$(ObjectSuffix): Components/CPlayer.cpp $(IntermediateDirectory)/Components_CPlayer$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Components/CPlayer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Components_CPlayer$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Components_CPlayer$(DependSuffix): Components/CPlayer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Components_CPlayer$(ObjectSuffix) -MF$(IntermediateDirectory)/Components_CPlayer$(DependSuffix) -MM "Components/CPlayer.cpp"

$(IntermediateDirectory)/Components_CPlayer$(PreprocessSuffix): Components/CPlayer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Components_CPlayer$(PreprocessSuffix) "Components/CPlayer.cpp"

$(IntermediateDirectory)/Components_CWall$(ObjectSuffix): Components/CWall.cpp $(IntermediateDirectory)/Components_CWall$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Components/CWall.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Components_CWall$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Components_CWall$(DependSuffix): Components/CWall.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Components_CWall$(ObjectSuffix) -MF$(IntermediateDirectory)/Components_CWall$(DependSuffix) -MM "Components/CWall.cpp"

$(IntermediateDirectory)/Components_CWall$(PreprocessSuffix): Components/CWall.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Components_CWall$(PreprocessSuffix) "Components/CWall.cpp"

$(IntermediateDirectory)/Global_Assets$(ObjectSuffix): Global/Assets.cpp $(IntermediateDirectory)/Global_Assets$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Global/Assets.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Global_Assets$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Global_Assets$(DependSuffix): Global/Assets.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Global_Assets$(ObjectSuffix) -MF$(IntermediateDirectory)/Global_Assets$(DependSuffix) -MM "Global/Assets.cpp"

$(IntermediateDirectory)/Global_Assets$(PreprocessSuffix): Global/Assets.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Global_Assets$(PreprocessSuffix) "Global/Assets.cpp"

$(IntermediateDirectory)/Global_Config$(ObjectSuffix): Global/Config.cpp $(IntermediateDirectory)/Global_Config$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Global/Config.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Global_Config$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Global_Config$(DependSuffix): Global/Config.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Global_Config$(ObjectSuffix) -MF$(IntermediateDirectory)/Global_Config$(DependSuffix) -MM "Global/Config.cpp"

$(IntermediateDirectory)/Global_Config$(PreprocessSuffix): Global/Config.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Global_Config$(PreprocessSuffix) "Global/Config.cpp"

$(IntermediateDirectory)/Global_Factory$(ObjectSuffix): Global/Factory.cpp $(IntermediateDirectory)/Global_Factory$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Global/Factory.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Global_Factory$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Global_Factory$(DependSuffix): Global/Factory.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Global_Factory$(ObjectSuffix) -MF$(IntermediateDirectory)/Global_Factory$(DependSuffix) -MM "Global/Factory.cpp"

$(IntermediateDirectory)/Global_Factory$(PreprocessSuffix): Global/Factory.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Global_Factory$(PreprocessSuffix) "Global/Factory.cpp"

$(IntermediateDirectory)/Data_StyleData$(ObjectSuffix): Data/StyleData.cpp $(IntermediateDirectory)/Data_StyleData$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Data/StyleData.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Data_StyleData$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Data_StyleData$(DependSuffix): Data/StyleData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Data_StyleData$(ObjectSuffix) -MF$(IntermediateDirectory)/Data_StyleData$(DependSuffix) -MM "Data/StyleData.cpp"

$(IntermediateDirectory)/Data_StyleData$(PreprocessSuffix): Data/StyleData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Data_StyleData$(PreprocessSuffix) "Data/StyleData.cpp"

$(IntermediateDirectory)/Data_LevelData$(ObjectSuffix): Data/LevelData.cpp $(IntermediateDirectory)/Data_LevelData$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Data/LevelData.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Data_LevelData$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Data_LevelData$(DependSuffix): Data/LevelData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Data_LevelData$(ObjectSuffix) -MF$(IntermediateDirectory)/Data_LevelData$(DependSuffix) -MM "Data/LevelData.cpp"

$(IntermediateDirectory)/Data_LevelData$(PreprocessSuffix): Data/LevelData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Data_LevelData$(PreprocessSuffix) "Data/LevelData.cpp"

$(IntermediateDirectory)/Data_ProfileData$(ObjectSuffix): Data/ProfileData.cpp $(IntermediateDirectory)/Data_ProfileData$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Data/ProfileData.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Data_ProfileData$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Data_ProfileData$(DependSuffix): Data/ProfileData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Data_ProfileData$(ObjectSuffix) -MF$(IntermediateDirectory)/Data_ProfileData$(DependSuffix) -MM "Data/ProfileData.cpp"

$(IntermediateDirectory)/Data_ProfileData$(PreprocessSuffix): Data/ProfileData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Data_ProfileData$(PreprocessSuffix) "Data/ProfileData.cpp"

$(IntermediateDirectory)/Data_EventData$(ObjectSuffix): Data/EventData.cpp $(IntermediateDirectory)/Data_EventData$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Data/EventData.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Data_EventData$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Data_EventData$(DependSuffix): Data/EventData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Data_EventData$(ObjectSuffix) -MF$(IntermediateDirectory)/Data_EventData$(DependSuffix) -MM "Data/EventData.cpp"

$(IntermediateDirectory)/Data_EventData$(PreprocessSuffix): Data/EventData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Data_EventData$(PreprocessSuffix) "Data/EventData.cpp"

$(IntermediateDirectory)/Data_PackData$(ObjectSuffix): Data/PackData.cpp $(IntermediateDirectory)/Data_PackData$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Data/PackData.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Data_PackData$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Data_PackData$(DependSuffix): Data/PackData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Data_PackData$(ObjectSuffix) -MF$(IntermediateDirectory)/Data_PackData$(DependSuffix) -MM "Data/PackData.cpp"

$(IntermediateDirectory)/Data_PackData$(PreprocessSuffix): Data/PackData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Data_PackData$(PreprocessSuffix) "Data/PackData.cpp"

$(IntermediateDirectory)/Data_MusicData$(ObjectSuffix): Data/MusicData.cpp $(IntermediateDirectory)/Data_MusicData$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Data/MusicData.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Data_MusicData$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Data_MusicData$(DependSuffix): Data/MusicData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Data_MusicData$(ObjectSuffix) -MF$(IntermediateDirectory)/Data_MusicData$(DependSuffix) -MM "Data/MusicData.cpp"

$(IntermediateDirectory)/Data_MusicData$(PreprocessSuffix): Data/MusicData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Data_MusicData$(PreprocessSuffix) "Data/MusicData.cpp"

$(IntermediateDirectory)/Utils_Utils$(ObjectSuffix): Utils/Utils.cpp $(IntermediateDirectory)/Utils_Utils$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Utils/Utils.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Utils_Utils$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Utils_Utils$(DependSuffix): Utils/Utils.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Utils_Utils$(ObjectSuffix) -MF$(IntermediateDirectory)/Utils_Utils$(DependSuffix) -MM "Utils/Utils.cpp"

$(IntermediateDirectory)/Utils_Utils$(PreprocessSuffix): Utils/Utils.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Utils_Utils$(PreprocessSuffix) "Utils/Utils.cpp"

$(IntermediateDirectory)/Utils_MD5$(ObjectSuffix): Utils/MD5.cpp $(IntermediateDirectory)/Utils_MD5$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Utils/MD5.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Utils_MD5$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Utils_MD5$(DependSuffix): Utils/MD5.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Utils_MD5$(ObjectSuffix) -MF$(IntermediateDirectory)/Utils_MD5$(DependSuffix) -MM "Utils/MD5.cpp"

$(IntermediateDirectory)/Utils_MD5$(PreprocessSuffix): Utils/MD5.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Utils_MD5$(PreprocessSuffix) "Utils/MD5.cpp"

$(IntermediateDirectory)/Utils_FPSWatcher$(ObjectSuffix): Utils/FPSWatcher.cpp $(IntermediateDirectory)/Utils_FPSWatcher$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Utils/FPSWatcher.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Utils_FPSWatcher$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Utils_FPSWatcher$(DependSuffix): Utils/FPSWatcher.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Utils_FPSWatcher$(ObjectSuffix) -MF$(IntermediateDirectory)/Utils_FPSWatcher$(DependSuffix) -MM "Utils/FPSWatcher.cpp"

$(IntermediateDirectory)/Utils_FPSWatcher$(PreprocessSuffix): Utils/FPSWatcher.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Utils_FPSWatcher$(PreprocessSuffix) "Utils/FPSWatcher.cpp"

$(IntermediateDirectory)/Utils_Base64$(ObjectSuffix): Utils/Base64.cpp $(IntermediateDirectory)/Utils_Base64$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Utils/Base64.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Utils_Base64$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Utils_Base64$(DependSuffix): Utils/Base64.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Utils_Base64$(ObjectSuffix) -MF$(IntermediateDirectory)/Utils_Base64$(DependSuffix) -MM "Utils/Base64.cpp"

$(IntermediateDirectory)/Utils_Base64$(PreprocessSuffix): Utils/Base64.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Utils_Base64$(PreprocessSuffix) "Utils/Base64.cpp"

$(IntermediateDirectory)/Online_Online$(ObjectSuffix): Online/Online.cpp $(IntermediateDirectory)/Online_Online$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Online/Online.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Online_Online$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Online_Online$(DependSuffix): Online/Online.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Online_Online$(ObjectSuffix) -MF$(IntermediateDirectory)/Online_Online$(DependSuffix) -MM "Online/Online.cpp"

$(IntermediateDirectory)/Online_Online$(PreprocessSuffix): Online/Online.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Online_Online$(PreprocessSuffix) "Online/Online.cpp"

$(IntermediateDirectory)/Compatibility_Compatibility$(ObjectSuffix): Compatibility/Compatibility.cpp $(IntermediateDirectory)/Compatibility_Compatibility$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Compatibility/Compatibility.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Compatibility_Compatibility$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Compatibility_Compatibility$(DependSuffix): Compatibility/Compatibility.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Compatibility_Compatibility$(ObjectSuffix) -MF$(IntermediateDirectory)/Compatibility_Compatibility$(DependSuffix) -MM "Compatibility/Compatibility.cpp"

$(IntermediateDirectory)/Compatibility_Compatibility$(PreprocessSuffix): Compatibility/Compatibility.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Compatibility_Compatibility$(PreprocessSuffix) "Compatibility/Compatibility.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) $(IntermediateDirectory)/main$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/main$(DependSuffix)
	$(RM) $(IntermediateDirectory)/main$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/HexagonGame$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/HexagonGame$(DependSuffix)
	$(RM) $(IntermediateDirectory)/HexagonGame$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/MenuGame$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/MenuGame$(DependSuffix)
	$(RM) $(IntermediateDirectory)/MenuGame$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/HGScripting$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/HGScripting$(DependSuffix)
	$(RM) $(IntermediateDirectory)/HGScripting$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/HGUpdate$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/HGUpdate$(DependSuffix)
	$(RM) $(IntermediateDirectory)/HGUpdate$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/HGProperties$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/HGProperties$(DependSuffix)
	$(RM) $(IntermediateDirectory)/HGProperties$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/HGGraphics$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/HGGraphics$(DependSuffix)
	$(RM) $(IntermediateDirectory)/HGGraphics$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Components_CPlayer$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Components_CPlayer$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Components_CPlayer$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Components_CWall$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Components_CWall$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Components_CWall$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Global_Assets$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Global_Assets$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Global_Assets$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Global_Config$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Global_Config$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Global_Config$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Global_Factory$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Global_Factory$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Global_Factory$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Data_StyleData$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Data_StyleData$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Data_StyleData$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Data_LevelData$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Data_LevelData$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Data_LevelData$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Data_ProfileData$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Data_ProfileData$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Data_ProfileData$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Data_EventData$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Data_EventData$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Data_EventData$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Data_PackData$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Data_PackData$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Data_PackData$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Data_MusicData$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Data_MusicData$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Data_MusicData$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Utils_Utils$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Utils_Utils$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Utils_Utils$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Utils_MD5$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Utils_MD5$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Utils_MD5$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Utils_FPSWatcher$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Utils_FPSWatcher$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Utils_FPSWatcher$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Utils_Base64$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Utils_Base64$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Utils_Base64$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Online_Online$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Online_Online$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Online_Online$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Compatibility_Compatibility$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Compatibility_Compatibility$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Compatibility_Compatibility$(PreprocessSuffix)
	$(RM) $(OutputFile)
	$(RM) $(OutputFile).exe
	$(RM) "../.build-release/SSVOpenHexagon"


