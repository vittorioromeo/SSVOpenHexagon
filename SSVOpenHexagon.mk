##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Release
ProjectName            :=SSVOpenHexagon
ConfigurationName      :=Release
WorkspacePath          := "D:\Vee\Software\GitHub\OHWorkspace"
ProjectPath            := "D:\Vee\Software\GitHub\OHWorkspace\SSVOpenHexagon"
IntermediateDirectory  :=./Release
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Vittorio
Date                   :=11/19/12
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
ObjectsFileList        :="D:\Vee\Software\GitHub\OHWorkspace\SSVOpenHexagon\SSVOpenHexagon.txt"
PCHCompileFlags        :=
MakeDirCommand         :=makedir
RcCmpOptions           := 
RcCompilerName         :=windres
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch)../SSVEntitySystem $(IncludeSwitch)../SSVStart $(IncludeSwitch)D:/Vee/Software/WIP/SFMLMinGW/include $(IncludeSwitch)D:/Vee/Software/WIP/jsoncpp/include 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)SSVEntitySystem $(LibrarySwitch)SSVStart $(LibrarySwitch)sfml-window $(LibrarySwitch)sfml-graphics $(LibrarySwitch)sfml-system $(LibrarySwitch)sfml-audio $(LibrarySwitch)json_mingw_libmt 
ArLibs                 :=  "SSVEntitySystem" "SSVStart" "sfml-window" "sfml-graphics" "sfml-system" "sfml-audio" "json_mingw_libmt" 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)../SSVStart/Release $(LibraryPathSwitch)../SSVEntitySystem/Release $(LibraryPathSwitch)D:/Vee/Software/WIP/SFMLMinGW/lib $(LibraryPathSwitch)D:/Vee/Software/WIP/jsoncpp/libs/mingw 

##
## Common variables
## AR, CXX, CC, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := ar rcus
CXX      := g++
CC       := gcc
CXXFLAGS :=  -O3 -pedantic -Wall -c -std=c++11 -Wextra  $(Preprocessors)
CFLAGS   :=  -O2 -Wall $(Preprocessors)


##
## User defined environment variables
##
CodeLiteDir:=c:\Program Files (x86)\CodeLite
WXWIN:=C:\wxWidgets
UNIT_TEST_PP_SRC_DIR:=C:\UnitTest++-1.3
WXCFG:=gcc_dll\mswu
Objects=$(IntermediateDirectory)/main$(ObjectSuffix) $(IntermediateDirectory)/HexagonGame$(ObjectSuffix) $(IntermediateDirectory)/PatternManager$(ObjectSuffix) $(IntermediateDirectory)/MenuGame$(ObjectSuffix) $(IntermediateDirectory)/Components_CPlayer$(ObjectSuffix) $(IntermediateDirectory)/Components_CWall$(ObjectSuffix) $(IntermediateDirectory)/Global_Assets$(ObjectSuffix) $(IntermediateDirectory)/Global_Config$(ObjectSuffix) $(IntermediateDirectory)/Global_Factory$(ObjectSuffix) $(IntermediateDirectory)/Data_StyleData$(ObjectSuffix) \
	$(IntermediateDirectory)/Data_MusicData$(ObjectSuffix) $(IntermediateDirectory)/Data_LevelData$(ObjectSuffix) $(IntermediateDirectory)/Data_ProfileData$(ObjectSuffix) $(IntermediateDirectory)/Utils_HSL$(ObjectSuffix) $(IntermediateDirectory)/Utils_Utils$(ObjectSuffix) 

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
	@$(MakeDirCommand) "./Release"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/main$(ObjectSuffix): main.cpp $(IntermediateDirectory)/main$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/main$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/main$(DependSuffix): main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/main$(ObjectSuffix) -MF$(IntermediateDirectory)/main$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/main.cpp"

$(IntermediateDirectory)/main$(PreprocessSuffix): main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/main$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/main.cpp"

$(IntermediateDirectory)/HexagonGame$(ObjectSuffix): HexagonGame.cpp $(IntermediateDirectory)/HexagonGame$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/HexagonGame.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/HexagonGame$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/HexagonGame$(DependSuffix): HexagonGame.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/HexagonGame$(ObjectSuffix) -MF$(IntermediateDirectory)/HexagonGame$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/HexagonGame.cpp"

$(IntermediateDirectory)/HexagonGame$(PreprocessSuffix): HexagonGame.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/HexagonGame$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/HexagonGame.cpp"

$(IntermediateDirectory)/PatternManager$(ObjectSuffix): PatternManager.cpp $(IntermediateDirectory)/PatternManager$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/PatternManager.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/PatternManager$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/PatternManager$(DependSuffix): PatternManager.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/PatternManager$(ObjectSuffix) -MF$(IntermediateDirectory)/PatternManager$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/PatternManager.cpp"

$(IntermediateDirectory)/PatternManager$(PreprocessSuffix): PatternManager.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/PatternManager$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/PatternManager.cpp"

$(IntermediateDirectory)/MenuGame$(ObjectSuffix): MenuGame.cpp $(IntermediateDirectory)/MenuGame$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/MenuGame.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/MenuGame$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/MenuGame$(DependSuffix): MenuGame.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/MenuGame$(ObjectSuffix) -MF$(IntermediateDirectory)/MenuGame$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/MenuGame.cpp"

$(IntermediateDirectory)/MenuGame$(PreprocessSuffix): MenuGame.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/MenuGame$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/MenuGame.cpp"

$(IntermediateDirectory)/Components_CPlayer$(ObjectSuffix): Components/CPlayer.cpp $(IntermediateDirectory)/Components_CPlayer$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Components/CPlayer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Components_CPlayer$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Components_CPlayer$(DependSuffix): Components/CPlayer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Components_CPlayer$(ObjectSuffix) -MF$(IntermediateDirectory)/Components_CPlayer$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Components/CPlayer.cpp"

$(IntermediateDirectory)/Components_CPlayer$(PreprocessSuffix): Components/CPlayer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Components_CPlayer$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Components/CPlayer.cpp"

$(IntermediateDirectory)/Components_CWall$(ObjectSuffix): Components/CWall.cpp $(IntermediateDirectory)/Components_CWall$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Components/CWall.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Components_CWall$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Components_CWall$(DependSuffix): Components/CWall.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Components_CWall$(ObjectSuffix) -MF$(IntermediateDirectory)/Components_CWall$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Components/CWall.cpp"

$(IntermediateDirectory)/Components_CWall$(PreprocessSuffix): Components/CWall.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Components_CWall$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Components/CWall.cpp"

$(IntermediateDirectory)/Global_Assets$(ObjectSuffix): Global/Assets.cpp $(IntermediateDirectory)/Global_Assets$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Global/Assets.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Global_Assets$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Global_Assets$(DependSuffix): Global/Assets.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Global_Assets$(ObjectSuffix) -MF$(IntermediateDirectory)/Global_Assets$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Global/Assets.cpp"

$(IntermediateDirectory)/Global_Assets$(PreprocessSuffix): Global/Assets.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Global_Assets$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Global/Assets.cpp"

$(IntermediateDirectory)/Global_Config$(ObjectSuffix): Global/Config.cpp $(IntermediateDirectory)/Global_Config$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Global/Config.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Global_Config$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Global_Config$(DependSuffix): Global/Config.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Global_Config$(ObjectSuffix) -MF$(IntermediateDirectory)/Global_Config$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Global/Config.cpp"

$(IntermediateDirectory)/Global_Config$(PreprocessSuffix): Global/Config.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Global_Config$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Global/Config.cpp"

$(IntermediateDirectory)/Global_Factory$(ObjectSuffix): Global/Factory.cpp $(IntermediateDirectory)/Global_Factory$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Global/Factory.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Global_Factory$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Global_Factory$(DependSuffix): Global/Factory.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Global_Factory$(ObjectSuffix) -MF$(IntermediateDirectory)/Global_Factory$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Global/Factory.cpp"

$(IntermediateDirectory)/Global_Factory$(PreprocessSuffix): Global/Factory.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Global_Factory$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Global/Factory.cpp"

$(IntermediateDirectory)/Data_StyleData$(ObjectSuffix): Data/StyleData.cpp $(IntermediateDirectory)/Data_StyleData$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Data/StyleData.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Data_StyleData$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Data_StyleData$(DependSuffix): Data/StyleData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Data_StyleData$(ObjectSuffix) -MF$(IntermediateDirectory)/Data_StyleData$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Data/StyleData.cpp"

$(IntermediateDirectory)/Data_StyleData$(PreprocessSuffix): Data/StyleData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Data_StyleData$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Data/StyleData.cpp"

$(IntermediateDirectory)/Data_MusicData$(ObjectSuffix): Data/MusicData.cpp $(IntermediateDirectory)/Data_MusicData$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Data/MusicData.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Data_MusicData$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Data_MusicData$(DependSuffix): Data/MusicData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Data_MusicData$(ObjectSuffix) -MF$(IntermediateDirectory)/Data_MusicData$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Data/MusicData.cpp"

$(IntermediateDirectory)/Data_MusicData$(PreprocessSuffix): Data/MusicData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Data_MusicData$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Data/MusicData.cpp"

$(IntermediateDirectory)/Data_LevelData$(ObjectSuffix): Data/LevelData.cpp $(IntermediateDirectory)/Data_LevelData$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Data/LevelData.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Data_LevelData$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Data_LevelData$(DependSuffix): Data/LevelData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Data_LevelData$(ObjectSuffix) -MF$(IntermediateDirectory)/Data_LevelData$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Data/LevelData.cpp"

$(IntermediateDirectory)/Data_LevelData$(PreprocessSuffix): Data/LevelData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Data_LevelData$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Data/LevelData.cpp"

$(IntermediateDirectory)/Data_ProfileData$(ObjectSuffix): Data/ProfileData.cpp $(IntermediateDirectory)/Data_ProfileData$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Data/ProfileData.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Data_ProfileData$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Data_ProfileData$(DependSuffix): Data/ProfileData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Data_ProfileData$(ObjectSuffix) -MF$(IntermediateDirectory)/Data_ProfileData$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Data/ProfileData.cpp"

$(IntermediateDirectory)/Data_ProfileData$(PreprocessSuffix): Data/ProfileData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Data_ProfileData$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Data/ProfileData.cpp"

$(IntermediateDirectory)/Utils_HSL$(ObjectSuffix): Utils/HSL.cpp $(IntermediateDirectory)/Utils_HSL$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Utils/HSL.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Utils_HSL$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Utils_HSL$(DependSuffix): Utils/HSL.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Utils_HSL$(ObjectSuffix) -MF$(IntermediateDirectory)/Utils_HSL$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Utils/HSL.cpp"

$(IntermediateDirectory)/Utils_HSL$(PreprocessSuffix): Utils/HSL.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Utils_HSL$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Utils/HSL.cpp"

$(IntermediateDirectory)/Utils_Utils$(ObjectSuffix): Utils/Utils.cpp $(IntermediateDirectory)/Utils_Utils$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Utils/Utils.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Utils_Utils$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Utils_Utils$(DependSuffix): Utils/Utils.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Utils_Utils$(ObjectSuffix) -MF$(IntermediateDirectory)/Utils_Utils$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Utils/Utils.cpp"

$(IntermediateDirectory)/Utils_Utils$(PreprocessSuffix): Utils/Utils.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Utils_Utils$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Utils/Utils.cpp"


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
	$(RM) $(IntermediateDirectory)/PatternManager$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/PatternManager$(DependSuffix)
	$(RM) $(IntermediateDirectory)/PatternManager$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/MenuGame$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/MenuGame$(DependSuffix)
	$(RM) $(IntermediateDirectory)/MenuGame$(PreprocessSuffix)
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
	$(RM) $(IntermediateDirectory)/Data_MusicData$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Data_MusicData$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Data_MusicData$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Data_LevelData$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Data_LevelData$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Data_LevelData$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Data_ProfileData$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Data_ProfileData$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Data_ProfileData$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Utils_HSL$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Utils_HSL$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Utils_HSL$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Utils_Utils$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Utils_Utils$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Utils_Utils$(PreprocessSuffix)
	$(RM) $(OutputFile)
	$(RM) $(OutputFile).exe
	$(RM) "D:\Vee\Software\GitHub\OHWorkspace\.build-release\SSVOpenHexagon"


