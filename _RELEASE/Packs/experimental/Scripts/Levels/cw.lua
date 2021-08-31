-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "nextpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "evolutionpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
        if mKey == 0 then pAltBarrage(u_rndInt(2, 4), 2)
    elseif mKey == 1 then pMirrorSpiral(u_rndInt(2, 4), 0)
    elseif mKey == 2 then pBarrageSpiral(u_rndInt(0, 3), 1, 1)
    elseif mKey == 3 then pBarrageSpiral(u_rndInt(0, 2), 1.2, 2)
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
shuffle(keys)
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

    l_setPulseMin(77)
    l_setPulseMax(95)
    l_setPulseSpeed(1.95)
    l_setPulseSpeedR(0.51)
    l_setPulseDelayMax(13)

    l_setBeatPulseMax(17)
    l_setBeatPulseDelayMax(27.8)

    l_setSwapEnabled(true)
    l_addTracked("special", "special")

    l_setTutorialMode(true)
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
    p0x, p0y, p1x, p1y, p2x, p2y, p3x, p3y = cw_getVertexPos4(self.cwHandle)

    if  p0x < -1000 or p0x > 1000 or
        p1x < -1000 or p1x > 1000 or
        p2x < -1000 or p2x > 1000 or
        p3x < -1000 or p3x > 1000 or
        p0y < -1000 or p0y > 1000 or
        p1y < -1000 or p1y > 1000 or
        p2y < -1000 or p2y > 1000 or
        p3y < -1000 or p3y > 1000
    then
        self.dead = true
        return
    end

    cw_moveVertexPos4Same(self.cwHandle,
        self.velocity_x * mFrameTime,
        self.velocity_y * mFrameTime)
end


-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
    e_waitUntilS(2)
    e_messageAddImportant("test", 130)
end

function randomSign()
    if u_rndReal() > 0.5 then
        return -1
    else
        return 1
    end
end

function makeWall()
    cwHandle = cw_createDeadly()

    if u_rndReal() > 0.5 then
        x = u_rndInt(-600, 600)
        y = 1000 * randomSign()
    else
        x = 1000 * randomSign()
        y = u_rndInt(-600, 600)
    end

    wallSize = u_rndInt(35, 85)

    cw_setVertexPos4(cwHandle, x + wallSize,     y + wallSize,
                               x + wallSize,     y + wallSize * 2,
                               x + wallSize * 2, y + wallSize * 2,
                               x + wallSize * 2, y + wallSize)

    cw_setVertexColor4Same(cwHandle, 255, 0, 0, 175)

    fw = FloatingWall:new(cwHandle)

    if u_rndReal() > 0.5 then
        fw.velocity_x = u_rndInt(10, 15) * randomSign()
        fw.velocity_y = 0
    else
        fw.velocity_x = 0
        fw.velocity_y = u_rndInt(10, 15) * randomSign()
    end

    table.insert(floatingWalls, fw)
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
what = true
function onStep()
    if what then
        for i=0, 15000 do
            makeWall()
        end

        what = false
    end
end

-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
    ArrayRemoveIf(floatingWalls, function(t, i, j)
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
