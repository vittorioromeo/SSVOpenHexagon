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
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch)../SSVEntitySystem $(IncludeSwitch)../SSVStart $(IncludeSwitch)D:/Vee/Software/WIP/SFMLMinGW/include $(IncludeSwitch)D:/Vee/Software/WIP/jsoncpp/include $(IncludeSwitch)D:/Vee/Software/WIP/boost 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)SSVEntitySystem $(LibrarySwitch)SSVStart $(LibrarySwitch)sfml-window $(LibrarySwitch)sfml-graphics $(LibrarySwitch)sfml-system $(LibrarySwitch)sfml-audio $(LibrarySwitch)json_mingw_libmt $(LibrarySwitch)boost_system-mgw47-mt-1_52 $(LibrarySwitch)boost_filesystem-mgw47-mt-1_52 
ArLibs                 :=  "SSVEntitySystem" "SSVStart" "sfml-window" "sfml-graphics" "sfml-system" "sfml-audio" "json_mingw_libmt" "boost_system-mgw47-mt-1_52" "boost_filesystem-mgw47-mt-1_52" 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)../SSVStart/Release $(LibraryPathSwitch)../SSVEntitySystem/Release $(LibraryPathSwitch)D:/Vee/Software/WIP/SFMLMinGW/lib $(LibraryPathSwitch)D:/Vee/Software/WIP/jsoncpp/libs/mingw $(LibraryPathSwitch)D:/Vee/Software/WIP/boost/bin.v2/libs/system/build/gcc-mingw-4.7.2/release/link-static/threading-multi $(LibraryPathSwitch)D:/Vee/Software/WIP/boost/bin.v2/libs/filesystem/build/gcc-mingw-4.7.2/release/link-static/threading-multi 

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
Objects=$(IntermediateDirectory)/main$(ObjectSuffix) $(IntermediateDirectory)/HexagonGame$(ObjectSuffix) $(IntermediateDirectory)/PatternManager$(ObjectSuffix) $(IntermediateDirectory)/HSL$(ObjectSuffix) $(IntermediateDirectory)/Utils$(ObjectSuffix) $(IntermediateDirectory)/LevelData$(ObjectSuffix) $(IntermediateDirectory)/MusicData$(ObjectSuffix) $(IntermediateDirectory)/StyleData$(ObjectSuffix) $(IntermediateDirectory)/Assets$(ObjectSuffix) $(IntermediateDirectory)/Config$(ObjectSuffix) \
	$(IntermediateDirectory)/Factory$(ObjectSuffix) $(IntermediateDirectory)/CPlayer$(ObjectSuffix) $(IntermediateDirectory)/CWall$(ObjectSuffix) $(IntermediateDirectory)/MenuGame$(ObjectSuffix) 

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

$(IntermediateDirectory)/HSL$(ObjectSuffix): HSL.cpp $(IntermediateDirectory)/HSL$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/HSL.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/HSL$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/HSL$(DependSuffix): HSL.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/HSL$(ObjectSuffix) -MF$(IntermediateDirectory)/HSL$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/HSL.cpp"

$(IntermediateDirectory)/HSL$(PreprocessSuffix): HSL.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/HSL$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/HSL.cpp"

$(IntermediateDirectory)/Utils$(ObjectSuffix): Utils.cpp $(IntermediateDirectory)/Utils$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Utils.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Utils$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Utils$(DependSuffix): Utils.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Utils$(ObjectSuffix) -MF$(IntermediateDirectory)/Utils$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Utils.cpp"

$(IntermediateDirectory)/Utils$(PreprocessSuffix): Utils.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Utils$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Utils.cpp"

$(IntermediateDirectory)/LevelData$(ObjectSuffix): LevelData.cpp $(IntermediateDirectory)/LevelData$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/LevelData.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/LevelData$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/LevelData$(DependSuffix): LevelData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/LevelData$(ObjectSuffix) -MF$(IntermediateDirectory)/LevelData$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/LevelData.cpp"

$(IntermediateDirectory)/LevelData$(PreprocessSuffix): LevelData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/LevelData$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/LevelData.cpp"

$(IntermediateDirectory)/MusicData$(ObjectSuffix): MusicData.cpp $(IntermediateDirectory)/MusicData$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/MusicData.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/MusicData$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/MusicData$(DependSuffix): MusicData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/MusicData$(ObjectSuffix) -MF$(IntermediateDirectory)/MusicData$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/MusicData.cpp"

$(IntermediateDirectory)/MusicData$(PreprocessSuffix): MusicData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/MusicData$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/MusicData.cpp"

$(IntermediateDirectory)/StyleData$(ObjectSuffix): StyleData.cpp $(IntermediateDirectory)/StyleData$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/StyleData.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/StyleData$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/StyleData$(DependSuffix): StyleData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/StyleData$(ObjectSuffix) -MF$(IntermediateDirectory)/StyleData$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/StyleData.cpp"

$(IntermediateDirectory)/StyleData$(PreprocessSuffix): StyleData.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/StyleData$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/StyleData.cpp"

$(IntermediateDirectory)/Assets$(ObjectSuffix): Assets.cpp $(IntermediateDirectory)/Assets$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Assets.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Assets$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Assets$(DependSuffix): Assets.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Assets$(ObjectSuffix) -MF$(IntermediateDirectory)/Assets$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Assets.cpp"

$(IntermediateDirectory)/Assets$(PreprocessSuffix): Assets.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Assets$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Assets.cpp"

$(IntermediateDirectory)/Config$(ObjectSuffix): Config.cpp $(IntermediateDirectory)/Config$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Config.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Config$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Config$(DependSuffix): Config.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Config$(ObjectSuffix) -MF$(IntermediateDirectory)/Config$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Config.cpp"

$(IntermediateDirectory)/Config$(PreprocessSuffix): Config.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Config$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Config.cpp"

$(IntermediateDirectory)/Factory$(ObjectSuffix): Factory.cpp $(IntermediateDirectory)/Factory$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Factory.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Factory$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Factory$(DependSuffix): Factory.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Factory$(ObjectSuffix) -MF$(IntermediateDirectory)/Factory$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Factory.cpp"

$(IntermediateDirectory)/Factory$(PreprocessSuffix): Factory.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Factory$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/Factory.cpp"

$(IntermediateDirectory)/CPlayer$(ObjectSuffix): CPlayer.cpp $(IntermediateDirectory)/CPlayer$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/CPlayer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/CPlayer$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/CPlayer$(DependSuffix): CPlayer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/CPlayer$(ObjectSuffix) -MF$(IntermediateDirectory)/CPlayer$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/CPlayer.cpp"

$(IntermediateDirectory)/CPlayer$(PreprocessSuffix): CPlayer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/CPlayer$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/CPlayer.cpp"

$(IntermediateDirectory)/CWall$(ObjectSuffix): CWall.cpp $(IntermediateDirectory)/CWall$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/CWall.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/CWall$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/CWall$(DependSuffix): CWall.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/CWall$(ObjectSuffix) -MF$(IntermediateDirectory)/CWall$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/CWall.cpp"

$(IntermediateDirectory)/CWall$(PreprocessSuffix): CWall.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/CWall$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/CWall.cpp"

$(IntermediateDirectory)/MenuGame$(ObjectSuffix): MenuGame.cpp $(IntermediateDirectory)/MenuGame$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/MenuGame.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/MenuGame$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/MenuGame$(DependSuffix): MenuGame.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/MenuGame$(ObjectSuffix) -MF$(IntermediateDirectory)/MenuGame$(DependSuffix) -MM "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/MenuGame.cpp"

$(IntermediateDirectory)/MenuGame$(PreprocessSuffix): MenuGame.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/MenuGame$(PreprocessSuffix) "D:/Vee/Software/GitHub/OHWorkspace/SSVOpenHexagon/MenuGame.cpp"


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
	$(RM) $(IntermediateDirectory)/HSL$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/HSL$(DependSuffix)
	$(RM) $(IntermediateDirectory)/HSL$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Utils$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Utils$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Utils$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/LevelData$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/LevelData$(DependSuffix)
	$(RM) $(IntermediateDirectory)/LevelData$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/MusicData$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/MusicData$(DependSuffix)
	$(RM) $(IntermediateDirectory)/MusicData$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/StyleData$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/StyleData$(DependSuffix)
	$(RM) $(IntermediateDirectory)/StyleData$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Assets$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Assets$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Assets$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Config$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Config$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Config$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/Factory$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/Factory$(DependSuffix)
	$(RM) $(IntermediateDirectory)/Factory$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/CPlayer$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/CPlayer$(DependSuffix)
	$(RM) $(IntermediateDirectory)/CPlayer$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/CWall$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/CWall$(DependSuffix)
	$(RM) $(IntermediateDirectory)/CWall$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/MenuGame$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/MenuGame$(DependSuffix)
	$(RM) $(IntermediateDirectory)/MenuGame$(PreprocessSuffix)
	$(RM) $(OutputFile)
	$(RM) $(OutputFile).exe
	$(RM) "D:\Vee\Software\GitHub\OHWorkspace\.build-release\SSVOpenHexagon"


