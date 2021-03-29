-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "nextpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "evolutionpatterns.lua")

FloatingWall = {}
FloatingWall.__index = FloatingWall

floatingWalls = {}

function FloatingWall:new(handle)
   local obj = {}
   setmetatable(obj, FloatingWall)
   obj.cwHandle = handle
   obj.dead = false
   obj.dir = 1
   return obj
end

function FloatingWall:move(mFrameTime)
    for i=0,3 do
        local oldX, oldY = cw_getVertexPos(self.cwHandle, i)

        if oldX < -1500 or oldX > 1500 or oldY < -1500 or oldY > 1500 then
            self.dead = true
            return
        end
    end

    cw_moveVertexPos4Same(self.cwHandle, self.velocity_x * mFrameTime,
                                        self.velocity_y * mFrameTime)

    if self.wobbly == true then
        self.velocity_y = self.velocity_y + ((self.dir * 0.10 * mFrameTime) * 4.0)

        if self.velocity_y >= 3 or self.velocity_y <= - 3 then
            self.dir = self.dir * -1
        end
    end
end

function mkVertWall(mY, mYVel, mX)
    local cwHandle = cw_createDeadly()

    local x = mX
    local y = mY

    local width = 3000
    local height = 40

    cw_setVertexPos4(cwHandle, x,         y,
                               x + width, y,
                               x + width, y + height,
                               x,         y + height)

    if nIncrement % 2 == 0 then
        cw_setVertexColor4Same(cwHandle, 0, 255, 0, 255)
    else
        cw_setVertexColor4Same(cwHandle, 255, 255, 255, 255)
    end

    local fw = FloatingWall:new(cwHandle)

    fw.wobbly = false
    fw.velocity_x = 0
    fw.velocity_y = mYVel * 1.5 * u_getDifficultyMult()

    table.insert(floatingWalls, fw)
end

function mkHalfHorizWall(mInv, mY, color, mXVel, mX)
    local cwHandle = cw_createDeadly()

    local x = mX
    local y = mY

    local width = 20
    local height = 800

    cw_setVertexPos4(cwHandle, x,         y,
                               x + width, y,
                               x + width, y + height,
                               x,         y + height)

    if nIncrement % 2 == 0 then
        cw_setVertexColor4Same(cwHandle, 0, 255, 0, 255)
    else
        cw_setVertexColor4Same(cwHandle, 255, 255, 255, 255)
    end

    local fw = FloatingWall:new(cwHandle)

    fw.wobbly = true
    fw.velocity_x = mXVel * 1.5 * u_getDifficultyMult()
    fw.velocity_y = 0

    table.insert(floatingWalls, fw)
end

waitTime = 3.1

upOffsets   = {0,  12.5, 25, 37.5, 50,  75,  150}
downOffsets = {50, 62.5, 75, 125,  200, 235, 270}

upOffsets2   = {0,  12.5, 25, 37.5, 50,  75,  150}
downOffsets2 = {50, 62.5, 75, 125,  200, 235, 270}

gapMod = 28;

function halfWaveImpl(spawnX, vel, colorA, colorB)
    local gap = -100 + gapMod;
    local yOff = 0 - gapMod / 2;

    for i=1, #upOffsets do
        t_eval([[mkHalfHorizWall(0, ]] .. yOff + downOffsets[i] + gap .. [[ , ]] .. colorA .. [[, ]] .. vel .. [[, ]] .. spawnX .. [[)
                 mkHalfHorizWall(0, ]] .. yOff + upOffsets[i]   .. [[ - 950, ]] .. colorB .. [[, ]] .. vel .. [[, ]] .. spawnX .. [[)]])
        t_wait(waitTime)
    end

    for i=#upOffsets - 1, 1, -1 do
        t_eval([[mkHalfHorizWall(0, ]] .. yOff + downOffsets[i] + gap .. [[ , ]] .. colorA .. [[, ]] .. vel .. [[, ]] .. spawnX .. [[)
                 mkHalfHorizWall(0, ]] .. yOff + upOffsets[i]   .. [[ - 950, ]] .. colorB .. [[, ]] .. vel .. [[, ]] .. spawnX .. [[)]])
        t_wait(waitTime)
    end
end

function halfWaveImplR(spawnX, vel, colorA, colorB)
    local gap = -100 + gapMod;
    local yOff = 0 - gapMod / 2;

    local off = 200;

    for i=1, #upOffsets do
        t_eval([[mkHalfHorizWall(0, ]] .. yOff + off - upOffsets2[i]   + gap .. [[ , ]] .. colorA .. [[, ]] .. vel .. [[, ]] .. spawnX .. [[)
                 mkHalfHorizWall(0, ]] .. yOff + off - downOffsets2[i] .. [[ - 950, ]] .. colorB .. [[, ]] .. vel .. [[, ]] .. spawnX .. [[)]])
        t_wait(waitTime)
    end

    for i=#upOffsets - 1, 1, -1 do
        t_eval([[mkHalfHorizWall(0, ]] .. yOff + off - upOffsets2[i]   + gap .. [[ , ]] .. colorA .. [[, ]] .. vel .. [[, ]] .. spawnX .. [[)
                 mkHalfHorizWall(0, ]] .. yOff + off - downOffsets2[i] .. [[ - 950, ]] .. colorB .. [[, ]] .. vel .. [[, ]] .. spawnX .. [[)]])
        t_wait(waitTime)
    end
end

function waveImpl(spawnX, vel, colorA, colorB)
    local gap = -100 + gapMod;
    local yOff = 0 - gapMod / 2;

    halfWaveImpl(spawnX, vel, colorA, colorB)

    for i=1, #upOffsets do
        t_eval([[mkHalfHorizWall(0, ]] .. yOff + 0 + gap .. [[ + 200, ]] .. colorA .. [[, ]] .. vel .. [[, ]] .. spawnX .. [[)
                 mkHalfHorizWall(0, ]] .. yOff + 0 .. [[ - 950, ]] .. colorB .. [[, ]] .. vel .. [[, ]] .. spawnX .. [[)]])
        t_wait(waitTime)
    end

    halfWaveImplR(spawnX, vel, colorA, colorB)

    for i=1, #upOffsets do
        t_eval([[mkHalfHorizWall(0, ]] .. yOff + 0 + gap .. [[ + 200, ]] .. colorA .. [[, ]] .. vel .. [[, ]] .. spawnX .. [[)
                 mkHalfHorizWall(0, ]] .. yOff + 0 .. [[ - 950, ]] .. colorB .. [[, ]] .. vel .. [[, ]] .. spawnX .. [[)]])
        t_wait(waitTime)
    end
end

function halfWaveLtoR()
    halfWaveImpl(-1000, 10, 0, 1)
end

function halfWaveRtoL()
    halfWaveImpl(1000, -10, 1, 0)
end

function revHalfWaveLtoR()
    halfWaveImplR(-1000, 10, 0, 1)
end

function revHalfWaveRtoL()
    halfWaveImplR(1000, -10, 1, 0)
end

function waveLtoR()
    waveImpl(-1000, 10, 0, 1)
end

function waveRtoL()
    waveImpl(1000, -10, 1, 0)
end

function pattern0()
    halfWaveLtoR()
    t_wait(waitTime * 3)
    halfWaveRtoL()
    t_wait(waitTime * 3)
end

function pattern1()
    waveLtoR()
    t_wait(waitTime * 3)
    waveRtoL()
    t_wait(waitTime * 3)
end

function pattern2()
    halfWaveRtoL()
    t_wait(waitTime * 3)
    halfWaveLtoR()
    t_wait(waitTime * 3)
end

function pattern3()
    waveRtoL()
    t_wait(waitTime * 3)
    waveLtoR()
    t_wait(waitTime * 3)
end

function pattern4()
    t_wait(30)
    t_eval([[mkVertWall(-1000, 10, -1500)]])
    t_wait(30)
end

function pattern5()
    t_wait(30)
    t_eval([[mkVertWall(1000, -10, -1500)]])
    t_wait(30)
end

function pattern6()
    revHalfWaveRtoL()
    t_wait(waitTime * 3)
    revHalfWaveLtoR()
    t_wait(waitTime * 3)
end

function pattern7()
    halfWaveLtoR()
    t_wait(waitTime * 3)
    halfWaveRtoL()
    t_wait(waitTime * 3)
end

function pattern8()
    revHalfWaveLtoR()
    t_wait(waitTime * 3)
    revHalfWaveRtoL()
    t_wait(waitTime * 3)
end

function pattern9()
    waveLtoR()
    waveRtoL()
end

function pattern10()
    halfWaveLtoR()
    t_wait(waitTime * 6)
    revHalfWaveRtoL()
    t_wait(waitTime * 12)
end

function pattern11()
    revHalfWaveLtoR()
    t_wait(waitTime * 6)
    halfWaveRtoL()
    t_wait(waitTime * 12)
end

function pattern12()
    halfWaveRtoL()
    t_wait(waitTime * 6)
    revHalfWaveLtoR()
    t_wait(waitTime * 12)
end

function pattern13()
    revHalfWaveRtoL()
    t_wait(waitTime * 6)
    halfWaveLtoR()
    t_wait(waitTime * 12)
end

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
        if mKey == 0 then pattern0()
    elseif mKey == 1 then pattern1()
    elseif mKey == 2 then pattern2()
    elseif mKey == 3 then pattern3()
    elseif mKey == 4 then pattern4() -- swap
    elseif mKey == 5 then pattern5() -- swap
    elseif mKey == 6 then pattern6()
    elseif mKey == 7 then pattern7()
    elseif mKey == 8 then pattern8()
    elseif mKey == 9 then pattern9()
    elseif mKey == 10 then pattern10()
    elseif mKey == 11 then pattern11()
    elseif mKey == 12 then pattern12()
    elseif mKey == 13 then pattern13()
    end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 1, 2, 3, 4, 5, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
         0, 1, 2, 3, 4, 5, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 }
shuffle(keys)
index = 0

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    l_setSpeedMult(0.0)
    l_setSpeedInc(0.0)

    l_setRotationSpeed(0.0)
    l_setRotationSpeedMax(1)
    l_setRotationSpeedInc(0.035)

    l_setDelayMult(0.0)
    l_setDelayInc(0.0)

    l_setFastSpin(0.0)

    l_setSides(36)
    l_setSidesMin(36)
    l_setSidesMax(36)

    l_setIncTime(15)

    l_setPulseMin(77)
    l_setPulseMax(95)
    l_setPulseSpeed(1.95)
    l_setPulseSpeedR(0.51)
    l_setPulseDelayMax(8)

    l_setBeatPulseMax(18)
    l_setBeatPulseDelayMax(26.67) -- BPM is 135
    l_setBeatPulseSpeedMult(0.45) -- Slows down the center going back to normal

    l_setSwapEnabled(true)

    if u_getDifficultyMult() >= 1.99 then
        l_setSwapCooldownMult(0.8)
        waitTime = 2.5
    elseif u_getDifficultyMult() >= 1.49 then
        l_setSwapCooldownMult(0.9)
        waitTime = 2.75
    elseif u_getDifficultyMult() <= 0.51 then
        l_setSwapCooldownMult(0.8)
        waitTime = 6
        gapMod = 75
    end
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
    e_waitUntilS(2)
    -- e_messageAddImportant("test", 130)
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()
    addPattern(keys[index])
    t_wait(waitTime * 3)

    index = index + 1

    if index - 1 == #keys then
        index = 1
        shuffle(keys)
    end
end

nIncrement = 0
oldRotSpeed = 0
lastRot = 0

-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
    if nIncrement == 0 then
        s_setStyle("arcadia2")
        u_setFlashEffect(255)
    elseif nIncrement == 1 then
        s_setStyle("arcadia")
        l_setRotation(90)
        lastRot = 90
        u_setFlashEffect(255)
        oldRotSpeed = l_getRotationSpeed()
        l_setRotationSpeed(0)
    elseif nIncrement % 2 == 0 then
        s_setStyle("arcadia2")
        u_setFlashEffect(255)
        l_setRotationSpeed(oldRotSpeed + l_getRotationSpeedInc())
    elseif nIncrement % 2 == 1 then
        s_setStyle("arcadia")
        lastRot = lastRot + 90
        l_setRotation(lastRot)
        u_setFlashEffect(255)
        oldRotSpeed = l_getRotationSpeed()
        l_setRotationSpeed(0)
    end

    nIncrement = nIncrement + 1

    cw_clear()
    floatingWalls = {}
    t_clear()
    t_wait(waitTime * 3)
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

-- just for testing!
--[[
function onInput(mFrameTime, mMovement, mFocus, mSwap)
    u_log("movement: " .. tostring(mMovement))
    u_log("focus: " .. tostring(mFocus))
    u_log("swap: " .. tostring(mSwap))

    -- prevent clockwise rotation
    if mMovement == 1 then
        return false
    end

    return true
end
]]--
