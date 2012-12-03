execScript("common.lua")
execScript("utils.lua")
execScript("alternativepatterns.lua")

function pTrap()
	if getSides() == 6 then
		patternizer({1,1,0,0,0,1,1,1,1,0,1,1,1,0,0,0,0,0,1,0,0,1,0,0,1,0,1,1,1,0,1,0,0,1,0,0,1,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0}, getPerfectThickness(THICKNESS))
	end
end