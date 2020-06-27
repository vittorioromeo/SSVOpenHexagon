-- include useful files
u_execScript("utils.lua")
u_execScript("common.lua")
u_execScript("commonpatterns.lua")
u_execScript("nextpatterns.lua")
u_execScript("evolutionpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
		if mKey == 0 then pAltBarrage(math.random(1, 3), 2)
	elseif mKey == 1 then pMirrorSpiral(math.random(2, 4), 0)
	elseif mKey == 2 then pBarrageSpiral(math.random(0, 3), 1, 1)
	elseif mKey == 3 then pBarrageSpiral(math.random(0, 2), 1.2, 2)
	elseif mKey == 4 then pBarrageSpiral(2, 0.7, 1)
	elseif mKey == 5 then pInverseBarrage(0)
	elseif mKey == 6 then hmcDefBarrageSpiral()
	elseif mKey == 7 then pMirrorWallStrip(1, 0)
	elseif mKey == 8 then hmcDefSpinner()
	elseif mKey == 9 then hmcDefBarrage()
	elseif mKey == 10 then hmcDef2Cage()
	elseif mKey == 11 then hmcDefBarrageSpiralSpin()
	end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 9, 10, 10, 8, 8, 9, 9, 9, 9, 6, 11, 11, 10, 10 }
keys = shuffle(keys)
index = 0

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
	l_setSpeedMult(1.7)
	l_setSpeedInc(0.15)
	l_setRotationSpeed(0.1)
	l_setRotationSpeedMax(0.4)
	l_setRotationSpeedInc(0.035)
	l_setDelayMult(1.2)
	l_setDelayInc(0.0)
	l_setFastSpin(0.0)
	l_setSides(6)
	l_setSidesMin(6)
	l_setSidesMax(6)
	l_setIncTime(15)
	l_setMaxInc(6)

	l_setPulseMin(77)
	l_setPulseMax(95)
	l_setPulseSpeed(1.95)
	l_setPulseSpeedR(0.51)
	l_setPulseDelayMax(13)

	l_setBeatPulseMax(17)
	l_setBeatPulseDelayMax(27.8)

	l_setSwapEnabled(true)
	l_addTracked("special", "special")
end

FloatingWall = {}
FloatingWall.__index = FloatingWall

floatingWalls = {}

function FloatingWall:new(handle)
   local obj = {}
   setmetatable(obj, FloatingWall)
   obj.cwHandle = cwHandle
   obj.dead = false
   return obj
end

function FloatingWall:move(mFrameTime)
	for i=0,3 do
		oldX,oldY = cw_getVertexPos(self.cwHandle, i)

		if oldX < -1000 or oldX > 1000 or oldY < -1000 or oldY > 1000 then
			self.dead = true
			return
		end

		cw_setVertexPos(self.cwHandle, i, oldX + self.velocity_x * mFrameTime, oldY + self.velocity_y * mFrameTime)
	end
end


-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()
	function randomSign()
		if math.random() > 0.5 then
			return -1
		else
			return 1
		end
	end

	cwHandle = cw_create()
	-- u_log("Created handle " .. cwHandle)

	if math.random() > 0.5 then
		x = math.random(-600, 600)
		y = 1000 * randomSign()
	else
		x = 1000 * randomSign()
		y = math.random(-600, 600)
	end

	wallSize = math.random(35, 85)

	cw_setVertexPos(cwHandle, 0, x + wallSize, y + wallSize)
	cw_setVertexPos(cwHandle, 1, x + wallSize, y + wallSize * 2)
	cw_setVertexPos(cwHandle, 2, x + wallSize * 2, y + wallSize * 2)
	cw_setVertexPos(cwHandle, 3, x + wallSize * 2, y + wallSize)

	cw_setVertexColor(cwHandle, 0, 255, 0, 0, 175)
	cw_setVertexColor(cwHandle, 1, 255, 0, 0, 175)
	cw_setVertexColor(cwHandle, 2, 255, 0, 0, 175)
	cw_setVertexColor(cwHandle, 3, 255, 0, 0, 175)

	fw = FloatingWall:new(cwHandle)

	if math.random() > 0.5 then
		fw.velocity_x = math.random(10, 15) * randomSign()
		fw.velocity_y = 0
	else
		fw.velocity_x = 0
		fw.velocity_y = math.random(10, 15) * randomSign()
	end

	table.insert(floatingWalls, fw)

	t_wait(1.5)
end

-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- TODO: move to utils
-- From: https://stackoverflow.com/questions/12394841/
function ArrayRemove(t, fnRemove)
    local j, n = 1, #t;

    for i=1,n do
        if (not fnRemove(t, i, j)) then
            -- Move i's kept value to j's position, if it's not already there.
            if (i ~= j) then
                t[j] = t[i];
                t[i] = nil;
            end
            j = j + 1; -- Increment position of where we'll place the next kept value.
        else
            t[i] = nil;
        end
    end

    return t;
end

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
	ArrayRemove(floatingWalls, function(t, i, j)
		local v = t[i]
		if v.dead then
			cw_destroy(v.cwHandle)
			return true
		else
			return false
		end
	end);

	for _, fw in ipairs(floatingWalls) do
		fw:move(mFrameTime)
	end
end
