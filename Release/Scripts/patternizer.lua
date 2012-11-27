execScript("common.lua")

function patternizer(mArray, myThickness)
	delay = getPerfectDelay(myThickness)
	eArray = cycle(getSides())
	j = math.floor(table.getn(mArray) / getSides())

	for i = 1, j do
		for k = 1, getSides() do
			if mArray[(i - 1)*getSides() + k] == 1 then
				wall(eArray[k], myThickness)
			end
		end
		wait(delay)
	end
end