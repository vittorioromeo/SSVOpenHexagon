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
Date                   :=24/03/2013
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
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch)./include/ $(IncludeSwitch)../SSVStart/include/ $(IncludeSwitch)../SSVUtils/include/ $(IncludeSwitch)../SSVUtilsJson/include/ $(IncludeSwitch)../SSVEntitySystem/include/ $(IncludeSwitch)../SSVLuaWrapper/include/ $(IncludeSwitch)../SSVMenuSystem/include/ $(IncludeSwitch)../SFML/include $(IncludeSwitch)../jsoncpp/include $(IncludeSwitch)C:/lua/include 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)SSVUtils-s $(LibrarySwitch)SSVUtilsJson-s $(LibrarySwitch)SSVStart-s $(LibrarySwitch)SSVEntitySystem-s $(LibrarySwitch)SSVLuaWrapper-s $(LibrarySwitch)SSVMenuSystem-s $(LibrarySwitch)sfml-window $(LibrarySwitch)sfml-graphics $(LibrarySwitch)sfml-system $(LibrarySwitch)sfml-audio $(LibrarySwitch)sfml-network $(LibrarySwitch)lua51 $(LibrarySwitch)json_mingw_libmt 
ArLibs                 :=  "SSVUtils-s" "SSVUtilsJson-s" "SSVStart-s" "SSVEntitySystem-s" "SSVLuaWrapper-s" "SSVMenuSystem-s" "sfml-window" "sfml-graphics" "sfml-system" "sfml-audio" "sfml-network" "lua51" "json_mingw_libmt" 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)../SSVUtils/lib/ $(LibraryPathSwitch)../SSVUtilsJson/lib/ $(LibraryPathSwitch)../SSVStart/lib/ $(LibraryPathSwitch)../SSVEntitySystem/lib/ $(LibraryPathSwitch)../SSVLuaWrapper/lib/ $(LibraryPathSwitch)../SSVMenuSystem/lib/ $(LibraryPathSwitch)../SFML/build2/lib/ $(LibraryPathSwitch)../lua $(LibraryPathSwitch)../jsoncpp/libs/mingw/ 

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
Objects0=$(IntermediateDirectory)/Compatibility_Compatibility$(ObjectSuffix) $(IntermediateDirectory)/Components_CPlayer$(ObjectSuffix) $(IntermediateDirectory)/Components_CWall$(ObjectSuffix) $(IntermediateDirectory)/Core_HGGraphics$(ObjectSuffix) $(IntermediateDirectory)/Core_HGProperties$(ObjectSuffix) $(IntermediateDirectory)/Core_HGScripting$(ObjectSuffix) $(IntermediateDirectory)/Core_HGUpdate$(ObjectSuffix) $(IntermediateDirectory)/Core_HexagonGame$(ObjectSuffix) $(IntermediateDirectory)/Core_MenuGame$(ObjectSuffix) $(IntermediateDirectory)/Core_main$(ObjectSuffix) \
	$(IntermediateDirectory)/Data_EventData$(ObjectSuffix) $(IntermediateDirectory)/Data_LevelData$(ObjectSuffix) $(IntermediateDirectory)/Data_MusicData$(ObjectSuffix) $(IntermediateDirectory)/Data_PackData$(ObjectSuffix) $(IntermediateDirectory)/Data_ProfileData$(ObjectSuffix) $(IntermediateDirectory)/Data_StyleData$(ObjectSuffix) $(IntermediateDirectory)/Global_Assets$(ObjectSuffix) $(IntermediateDirectory)/Global_Config$(ObjectSuffix) $(IntermediateDirectory)/Global_Factory$(ObjectSuffix) $(IntermediateDirectory)/Online_Online$(ObjectSuffix) \
	$(IntermediateDirectory)/Utils_FPSWatcher$(ObjectSuffix) $(IntermediateDirectory)/Utils_Utils$(ObjectSuffix) 



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
$(IntermediateDirectory)/Compatibility_Compatibility$(ObjectSuffix): src/SSVOpenHexagon/Compatibility/Compatibility.cpp $(IntermediateDirectory)/Compatibility_Compatibility$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/src/SSVOpenHexagon/Compatibility/Compatibility.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Compatibility_Compatibility$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Compatibility_Compatibility$(DependSuffix): src/SSVOpenHexagon/Compatibility/Compatibility.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Compatibility_Compatibility$(ObjectSuffix) -MF$(IntermediateDirectory)/Compatibility_Compatibility$(DependSuffix) -MM "src/SSVOpenHexagon/Compatibility/Compatibility.cpp"

$(IntermediateDirectory)/Compatibility_Compatibility$(PreprocessSuffix): src/SSVOpenHexagon/Compatibility/Compatibility.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Compatibility_Compatibility$(PreprocessSuffix) "src/SSVOpenHexagon/Compatibility/Compatibility.cpp"

$(IntermediateDirectory)/Components_CPlayer$(ObjectSuffix): src/SSVOpenHexagon/Components/CPlayer.cpp $(IntermediateDirectory)/Components_CPlayer$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/src/SSVOpenHexagon/Components/CPlayer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Components_CPlayer$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Components_CPlayer$(DependSuffix): src/SSVOpenHexagon/Components/CPlayer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Components_CPlayer$(ObjectSuffix) -MF$(IntermediateDirectory)/Components_CPlayer$(DependSuffix) -MM "src/SSVOpenHexagon/Components/CPlayer.cpp"

$(IntermediateDirectory)/Components_CPlayer$(PreprocessSuffix): src/SSVOpenHexagon/Components/CPlayer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Components_CPlayer$(PreprocessSuffix) "src/SSVOpenHexagon/Components/CPlayer.cpp"

$(IntermediateDirectory)/Components_CWall$(ObjectSuffix): src/SSVOpenHexagon/Components/CWall.cpp $(IntermediateDirectory)/Components_CWall$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/src/SSVOpenHexagon/Components/CWall.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Components_CWall$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Components_CWall$(DependSuffix): src/SSVOpenHexagon/Components/CWall.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Components_CWall$(ObjectSuffix) -MF$(IntermediateDirectory)/Components_CWall$(DependSuffix) -MM "src/SSVOpenHexagon/Components/CWall.cpp"

$(IntermediateDirectory)/Components_CWall$(PreprocessSuffix): src/SSVOpenHexagon/Components/CWall.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Components_CWall$(PreprocessSuffix) "src/SSVOpenHexagon/Components/CWall.cpp"

$(IntermediateDirectory)/Core_HGGraphics$(ObjectSuffix): src/SSVOpenHexagon/Core/HGGraphics.cpp $(IntermediateDirectory)/Core_HGGraphics$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/src/SSVOpenHexagon/Core/HGGraphics.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_HGGraphics$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_HGGraphics$(DependSuffix): src/SSVOpenHexagon/Core/HGGraphics.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_HGGraphics$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_HGGraphics$(DependSuffix) -MM "src/SSVOpenHexagon/Core/HGGraphics.cpp"

$(IntermediateDirectory)/Core_HGGraphics$(PreprocessSuffix): src/SSVOpenHexagon/Core/HGGraphics.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_HGGraphics$(PreprocessSuffix) "src/SSVOpenHexagon/Core/HGGraphics.cpp"

$(IntermediateDirectory)/Core_HGProperties$(ObjectSuffix): src/SSVOpenHexagon/Core/HGProperties.cpp $(IntermediateDirectory)/Core_HGProperties$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/src/SSVOpenHexagon/Core/HGProperties.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_HGProperties$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_HGProperties$(DependSuffix): src/SSVOpenHexagon/Core/HGProperties.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_HGProperties$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_HGProperties$(DependSuffix) -MM "src/SSVOpenHexagon/Core/HGProperties.cpp"

$(IntermediateDirectory)/Core_HGProperties$(PreprocessSuffix): src/SSVOpenHexagon/Core/HGProperties.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_HGProperties$(PreprocessSuffix) "src/SSVOpenHexagon/Core/HGProperties.cpp"

$(IntermediateDirectory)/Core_HGScripting$(ObjectSuffix): src/SSVOpenHexagon/Core/HGScripting.cpp $(IntermediateDirectory)/Core_HGScripting$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/src/SSVOpenHexagon/Core/HGScripting.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_HGScripting$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_HGScripting$(DependSuffix): src/SSVOpenHexagon/Core/HGScripting.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_HGScripting$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_HGScripting$(DependSuffix) -MM "src/SSVOpenHexagon/Core/HGScripting.cpp"

$(IntermediateDirectory)/Core_HGScripting$(PreprocessSuffix): src/SSVOpenHexagon/Core/HGScripting.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_HGScripting$(PreprocessSuffix) "src/SSVOpenHexagon/Core/HGScripting.cpp"

$(IntermediateDirectory)/Core_HGUpdate$(ObjectSuffix): src/SSVOpenHexagon/Core/HGUpdate.cpp $(IntermediateDirectory)/Core_HGUpdate$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/src/SSVOpenHexagon/Core/HGUpdate.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_HGUpdate$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_HGUpdate$(DependSuffix): src/SSVOpenHexagon/Core/HGUpdate.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_HGUpdate$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_HGUpdate$(DependSuffix) -MM "src/SSVOpenHexagon/Core/HGUpdate.cpp"

$(IntermediateDirectory)/Core_HGUpdate$(PreprocessSuffix): src/SSVOpenHexagon/Core/HGUpdate.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_HGUpdate$(PreprocessSuffix) "src/SSVOpenHexagon/Core/HGUpdate.cpp"

$(IntermediateDirectory)/Core_HexagonGame$(ObjectSuffix): src/SSVOpenHexagon/Core/HexagonGame.cpp $(IntermediateDirectory)/Core_HexagonGame$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/src/SSVOpenHexagon/Core/HexagonGame.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_HexagonGame$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_HexagonGame$(DependSuffix): src/SSVOpenHexagon/Core/HexagonGame.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_HexagonGame$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_HexagonGame$(DependSuffix) -MM "src/SSVOpenHexagon/Core/HexagonGame.cpp"

$(IntermediateDirectory)/Core_HexagonGame$(PreprocessSuffix): src/SSVOpenHexagon/Core/HexagonGame.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_HexagonGame$(PreprocessSuffix) "src/SSVOpenHexagon/Core/HexagonGame.cpp"

$(IntermediateDirectory)/Core_MenuGame$(ObjectSuffix): src/SSVOpenHexagon/Core/MenuGame.cpp $(IntermediateDirectory)/Core_MenuGame$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/src/SSVOpenHexagon/Core/MenuGame.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_MenuGame$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_MenuGame$(DependSuffix): src/SSVOpenHexagon/Core/MenuGame.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_MenuGame$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_MenuGame$(DependSuffix) -MM "src/SSVOpenHexagon/Core/MenuGame.cpp"

$(IntermediateDirectory)/Core_MenuGame$(PreprocessSuffix): src/SSVOpenHexagon/Core/MenuGame.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_MenuGame$(PreprocessSuffix) "src/SSVOpenHexagon/Core/MenuGame.cpp"

$(IntermediateDirectory)/Core_main$(ObjectSuffix): src/SSVOpenHexagon/Core/main.cpp $(IntermediateDirectory)/Core_main$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/src/SSVOpenHexagon/Core/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Core_main$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Core_main$(DependSuffix): src/SSVOpenHexagon/Core/main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Core_main$(ObjectSuffix) -MF$(IntermediateDirectory)/Core_main$(DependSuffix) -MM "src/SSVOpenHexagon/Core/main.cpp"

$(IntermediateDirectory)/Core_main$(PreprocessSuffix): src/SSVOpenHexagon/Core/main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Core_main$(PreprocessSuffix) "src/SSVOpenHexagon/Core/main.cpp"

$(IntermediateDirectory)/Data_EventData$(ObjectSuffix): src/SSVOpenHexagon/Data/EventData.cpp $(IntermediateDirectory)/Data_EventData$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/src/SSVOpenHexagon/Data/EventData.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Data_EventData$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Data_EventData$(DependSuffix): src/SSVOpenHexagon/Data/EventData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Data_EventData$(ObjectSuffix) -MF$(IntermediateDirectory)/Data_EventData$(DependSuffix) -MM "src/SSVOpenHexagon/Data/EventData.cpp"

$(IntermediateDirectory)/Data_EventData$(PreprocessSuffix): src/SSVOpenHexagon/Data/EventData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Data_EventData$(PreprocessSuffix) "src/SSVOpenHexagon/Data/EventData.cpp"

$(IntermediateDirectory)/Data_LevelData$(ObjectSuffix): src/SSVOpenHexagon/Data/LevelData.cpp $(IntermediateDirectory)/Data_LevelData$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/src/SSVOpenHexagon/Data/LevelData.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Data_LevelData$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Data_LevelData$(DependSuffix): src/SSVOpenHexagon/Data/LevelData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Data_LevelData$(ObjectSuffix) -MF$(IntermediateDirectory)/Data_LevelData$(DependSuffix) -MM "src/SSVOpenHexagon/Data/LevelData.cpp"

$(IntermediateDirectory)/Data_LevelData$(PreprocessSuffix): src/SSVOpenHexagon/Data/LevelData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Data_LevelData$(PreprocessSuffix) "src/SSVOpenHexagon/Data/LevelData.cpp"

$(IntermediateDirectory)/Data_MusicData$(ObjectSuffix): src/SSVOpenHexagon/Data/MusicData.cpp $(IntermediateDirectory)/Data_MusicData$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/src/SSVOpenHexagon/Data/MusicData.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Data_MusicData$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Data_MusicData$(DependSuffix): src/SSVOpenHexagon/Data/MusicData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Data_MusicData$(ObjectSuffix) -MF$(IntermediateDirectory)/Data_MusicData$(DependSuffix) -MM "src/SSVOpenHexagon/Data/MusicData.cpp"

$(IntermediateDirectory)/Data_MusicData$(PreprocessSuffix): src/SSVOpenHexagon/Data/MusicData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Data_MusicData$(PreprocessSuffix) "src/SSVOpenHexagon/Data/MusicData.cpp"

$(IntermediateDirectory)/Data_PackData$(ObjectSuffix): src/SSVOpenHexagon/Data/PackData.cpp $(IntermediateDirectory)/Data_PackData$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/src/SSVOpenHexagon/Data/PackData.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Data_PackData$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Data_PackData$(DependSuffix): src/SSVOpenHexagon/Data/PackData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Data_PackData$(ObjectSuffix) -MF$(IntermediateDirectory)/Data_PackData$(DependSuffix) -MM "src/SSVOpenHexagon/Data/PackData.cpp"

$(IntermediateDirectory)/Data_PackData$(PreprocessSuffix): src/SSVOpenHexagon/Data/PackData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Data_PackData$(PreprocessSuffix) "src/SSVOpenHexagon/Data/PackData.cpp"

$(IntermediateDirectory)/Data_ProfileData$(ObjectSuffix): src/SSVOpenHexagon/Data/ProfileData.cpp $(IntermediateDirectory)/Data_ProfileData$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/src/SSVOpenHexagon/Data/ProfileData.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Data_ProfileData$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Data_ProfileData$(DependSuffix): src/SSVOpenHexagon/Data/ProfileData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Data_ProfileData$(ObjectSuffix) -MF$(IntermediateDirectory)/Data_ProfileData$(DependSuffix) -MM "src/SSVOpenHexagon/Data/ProfileData.cpp"

$(IntermediateDirectory)/Data_ProfileData$(PreprocessSuffix): src/SSVOpenHexagon/Data/ProfileData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Data_ProfileData$(PreprocessSuffix) "src/SSVOpenHexagon/Data/ProfileData.cpp"

$(IntermediateDirectory)/Data_StyleData$(ObjectSuffix): src/SSVOpenHexagon/Data/StyleData.cpp $(IntermediateDirectory)/Data_StyleData$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/src/SSVOpenHexagon/Data/StyleData.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Data_StyleData$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Data_StyleData$(DependSuffix): src/SSVOpenHexagon/Data/StyleData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Data_StyleData$(ObjectSuffix) -MF$(IntermediateDirectory)/Data_StyleData$(DependSuffix) -MM "src/SSVOpenHexagon/Data/StyleData.cpp"

$(IntermediateDirectory)/Data_StyleData$(PreprocessSuffix): src/SSVOpenHexagon/Data/StyleData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Data_StyleData$(PreprocessSuffix) "src/SSVOpenHexagon/Data/StyleData.cpp"

$(IntermediateDirectory)/Global_Assets$(ObjectSuffix): src/SSVOpenHexagon/Global/Assets.cpp $(IntermediateDirectory)/Global_Assets$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/src/SSVOpenHexagon/Global/Assets.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Global_Assets$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Global_Assets$(DependSuffix): src/SSVOpenHexagon/Global/Assets.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Global_Assets$(ObjectSuffix) -MF$(IntermediateDirectory)/Global_Assets$(DependSuffix) -MM "src/SSVOpenHexagon/Global/Assets.cpp"

$(IntermediateDirectory)/Global_Assets$(PreprocessSuffix): src/SSVOpenHexagon/Global/Assets.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Global_Assets$(PreprocessSuffix) "src/SSVOpenHexagon/Global/Assets.cpp"

$(IntermediateDirectory)/Global_Config$(ObjectSuffix): src/SSVOpenHexagon/Global/Config.cpp $(IntermediateDirectory)/Global_Config$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/src/SSVOpenHexagon/Global/Config.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Global_Config$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Global_Config$(DependSuffix): src/SSVOpenHexagon/Global/Config.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Global_Config$(ObjectSuffix) -MF$(IntermediateDirectory)/Global_Config$(DependSuffix) -MM "src/SSVOpenHexagon/Global/Config.cpp"

$(IntermediateDirectory)/Global_Config$(PreprocessSuffix): src/SSVOpenHexagon/Global/Config.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Global_Config$(PreprocessSuffix) "src/SSVOpenHexagon/Global/Config.cpp"

$(IntermediateDirectory)/Global_Factory$(ObjectSuffix): src/SSVOpenHexagon/Global/Factory.cpp $(IntermediateDirectory)/Global_Factory$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/src/SSVOpenHexagon/Global/Factory.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Global_Factory$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Global_Factory$(DependSuffix): src/SSVOpenHexagon/Global/Factory.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Global_Factory$(ObjectSuffix) -MF$(IntermediateDirectory)/Global_Factory$(DependSuffix) -MM "src/SSVOpenHexagon/Global/Factory.cpp"

$(IntermediateDirectory)/Global_Factory$(PreprocessSuffix): src/SSVOpenHexagon/Global/Factory.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Global_Factory$(PreprocessSuffix) "src/SSVOpenHexagon/Global/Factory.cpp"

$(IntermediateDirectory)/Online_Online$(ObjectSuffix): src/SSVOpenHexagon/Online/Online.cpp $(IntermediateDirectory)/Online_Online$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/src/SSVOpenHexagon/Online/Online.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Online_Online$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Online_Online$(DependSuffix): src/SSVOpenHexagon/Online/Online.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Online_Online$(ObjectSuffix) -MF$(IntermediateDirectory)/Online_Online$(DependSuffix) -MM "src/SSVOpenHexagon/Online/Online.cpp"

$(IntermediateDirectory)/Online_Online$(PreprocessSuffix): src/SSVOpenHexagon/Online/Online.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Online_Online$(PreprocessSuffix) "src/SSVOpenHexagon/Online/Online.cpp"

$(IntermediateDirectory)/Utils_FPSWatcher$(ObjectSuffix): src/SSVOpenHexagon/Utils/FPSWatcher.cpp $(IntermediateDirectory)/Utils_FPSWatcher$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/src/SSVOpenHexagon/Utils/FPSWatcher.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Utils_FPSWatcher$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Utils_FPSWatcher$(DependSuffix): src/SSVOpenHexagon/Utils/FPSWatcher.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Utils_FPSWatcher$(ObjectSuffix) -MF$(IntermediateDirectory)/Utils_FPSWatcher$(DependSuffix) -MM "src/SSVOpenHexagon/Utils/FPSWatcher.cpp"

$(IntermediateDirectory)/Utils_FPSWatcher$(PreprocessSuffix): src/SSVOpenHexagon/Utils/FPSWatcher.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Utils_FPSWatcher$(PreprocessSuffix) "src/SSVOpenHexagon/Utils/FPSWatcher.cpp"

$(IntermediateDirectory)/Utils_Utils$(ObjectSuffix): src/SSVOpenHexagon/Utils/Utils.cpp $(IntermediateDirectory)/Utils_Utils$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/src/SSVOpenHexagon/Utils/Utils.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Utils_Utils$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Utils_Utils$(DependSuffix): src/SSVOpenHexagon/Utils/Utils.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Utils_Utils$(ObjectSuffix) -MF$(IntermediateDirectory)/Utils_Utils$(DependSuffix) -MM "src/SSVOpenHexagon/Utils/Utils.cpp"

$(IntermediateDirectory)/Utils_Utils$(PreprocessSuffix): src/SSVOpenHexagon/Utils/Utils.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Utils_Utils$(PreprocessSuffix) "src/SSVOpenHexagon/Utils/Utils.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) $(IntermediateDirectory)/Compatibility_Compatibility$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Compatibility_Compatibility$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Compatibility_Compatibility$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Components_CPlayer$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Components_CPlayer$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Components_CPlayer$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Components_CWall$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Components_CWall$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Components_CWall$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Core_HGGraphics$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_HGGraphics$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_HGGraphics$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Core_HGProperties$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_HGProperties$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_HGProperties$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Core_HGScripting$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_HGScripting$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_HGScripting$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Core_HGUpdate$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_HGUpdate$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_HGUpdate$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Core_HexagonGame$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_HexagonGame$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_HexagonGame$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Core_MenuGame$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_MenuGame$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_MenuGame$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Core_main$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Core_main$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Core_main$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Data_EventData$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Data_EventData$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Data_EventData$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Data_LevelData$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Data_LevelData$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Data_LevelData$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Data_MusicData$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Data_MusicData$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Data_MusicData$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Data_PackData$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Data_PackData$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Data_PackData$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Data_ProfileData$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Data_ProfileData$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Data_ProfileData$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Data_StyleData$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Data_StyleData$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Data_StyleData$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Global_Assets$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Global_Assets$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Global_Assets$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Global_Config$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Global_Config$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Global_Config$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Global_Factory$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Global_Factory$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Global_Factory$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Online_Online$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Online_Online$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Online_Online$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Utils_FPSWatcher$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Utils_FPSWatcher$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Utils_FPSWatcher$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Utils_Utils$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Utils_Utils$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Utils_Utils$(PreprocessSuffix)
	$(RM) $(OutputFile)
	$(RM) $(OutputFile).exe
	$(RM) "../.build-release/SSVOpenHexagon"


