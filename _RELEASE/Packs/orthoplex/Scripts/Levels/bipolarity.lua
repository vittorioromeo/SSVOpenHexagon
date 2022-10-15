-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "nextpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "evolutionpatterns.lua")

oldDirection = 0
direction = 0
changes = 0
style = 0
lastRotationDir = 0
swapped = false
rotSpeed = 0.25
rotSpeedMax = 0.9
achievementUnlocked = false
hardAchievementUnlocked = false

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
        local oldX,oldY = cw_getVertexPos(self.cwHandle, i)

        if oldX < -3500 or oldX > 3500 or oldY < -3500 or oldY > 3500 then
            self.dead = true
            return
        end

        cw_setVertexPos(self.cwHandle, i,
            oldX + self.velocity_x * mFrameTime,
            oldY + self.velocity_y * mFrameTime)

        if self.wobbly == true then
            self.velocity_y = self.velocity_y + (self.dir * 0.10 * mFrameTime)

            if self.velocity_y >= 3 or self.velocity_y <= - 3 then
                self.dir = self.dir * -1
            end
        end
    end
end

function rotatePoint(ac, as, cx, cy, angle, px, py)
    return ac * (px - cx) - as * (py - cy) + cx,
           as * (px - cx) + ac * (py - cy) + cy
end

function toRad(x)
    return x * math.pi / 180.0
end

function mkVertWall(mAngle, mY, mYVel, mX)
    local cwHandle = cw_createDeadly()

    mAngle = toRad(mAngle)

    local width = 4000
    local height = 35

    local cx = mX
    local cy = mY

    local px = mX - width / 2.0
    local py = mY - height / 2.0

    local ac = math.cos(mAngle)
    local as = math.sin(mAngle)

    local ac90 = math.cos(mAngle + math.pi / 2.0)
    local as90 = math.sin(mAngle + math.pi / 2.0)

    local x0, y0 = rotatePoint(ac, as, cx, cy, mAngle, px, py)
    local x1, y1 = rotatePoint(ac, as, cx, cy, mAngle, px + width, py)
    local x2, y2 = rotatePoint(ac, as, cx, cy, mAngle, px + width, py + height)
    local x3, y3 = rotatePoint(ac, as, cx, cy, mAngle, px, py + height)

    cw_setVertexPos4(cwHandle, x0, y0,
                               x1, y1,
                               x2, y2,
                               x3, y3)

    local fw = FloatingWall:new(cwHandle)

    fw.wobbly = false
    fw.velocity_x = mYVel * ac90 * 1.5 * (u_getDifficultyMult() ^ 0.25)
    fw.velocity_y = mYVel * as90 * 1.5 * (u_getDifficultyMult() ^ 0.25)

    table.insert(floatingWalls, fw)
end

function mkSwapWall(mSide)
    local inc = 360 / l_getSides()
    local angle = inc * mSide + 90
    local dist = 1400
    local spawnAngleRad = toRad(angle + 90 + 180)

    local x = dist * math.cos(spawnAngleRad)
    local y = dist * math.sin(spawnAngleRad)

    mkVertWall(angle, y, 20, x)
end

function setDirection(x)
    oldDirection = direction
    direction = math.floor(math.fmod(x, l_getSides()))
    if direction < 0 then
        direction = direction + l_getSides()
    end
end

maxChanges = 0

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
    dm = u_getDifficultyMult()
    changes = changes + 1

    if changes == maxChanges then

        maxChanges = u_rndInt(5, 12)
        changes = 0

        setDirection(direction + l_getSides() / 2+ getRandomDir())

        local oldT = THICKNESS
        THICKNESS = 160
        cBarrage(oldDirection + l_getSides() / 2)
        THICKNESS = oldT


        beat = getBPMToBeatPulseDelay(180) / getMusicDMSyncFactor()
        t_wait(beat)

        if (dm == 1.0) then
            e_wait(beat)
        elseif (dm == 0.5) then
            e_wait(beat * 2)
        end

        e_eval([[s_setCapColorMain()]])

        if style == 0 then
            e_eval([[u_setFlashColor(255, 255, 255)]])
            e_eval([[u_setFlashEffect(100)]])
        else
            e_eval([[u_setFlashColor(0, 0, 0)]])
            e_eval([[u_setFlashEffect(215)]])
        end

        e_wait(beat * 2)
        e_eval([[s_setCapColorByIndex(0)]])
        --        t_wait(10 * (dm ^ 0.2))

--        t_eval([[mkSwapWall(]] .. oldDirection + l_getSides() / 2 .. [[)]])

--        t_wait(10 * (dm ^ 0.2))
--        if dm < 1.0 then
--            t_wait(5)
--        end
    else
        local oldT = THICKNESS
        THICKNESS = 80
        cBarrage(direction)
        THICKNESS = oldT

        if changes ~= maxChanges - 1 then
            setDirection(direction + getRandomDir())
        end

        -- local delay = getPerfectDelay(THICKNESS) * 5.6
        -- t_wait(delay)

        beat = getBPMToBeatPulseDelay(180) / getMusicDMSyncFactor()
        t_wait(beat)
    end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 0, 0, 0, 0, 0, 0, 3, 4, 5, 5 }
shuffle(keys)
index = 0
achievementUnlocked = false
timeAcc = 0

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    l_setSpeedMult(5.75)
    l_setSpeedInc(0.25)
    l_setSpeedMax(7.75)
    l_setRotationSpeed(0.0)
    l_setRotationSpeedMax(1)
    l_setRotationSpeedInc(0.04)

    l_setDelayMult(1.00)

    l_setDelayInc(0)
    l_setFastSpin(0.0)
    l_setSides(6)
    l_setSidesMin(6)
    l_setSidesMax(6)
    l_setIncTime(15)

    local pulseOffset = 0

    if u_getDifficultyMult() > 1.5 then
        pulseOffset = -10
    end

    l_setPulseMin(75 + pulseOffset)
    l_setPulseMax(125 + pulseOffset)
    l_setPulseSpeed(5.388)
    l_setPulseSpeedR(5.353)
    l_setPulseDelayMax(21.38)

    l_setBeatPulseMax(35)
    beat = getBPMToBeatPulseDelay(180) / getMusicDMSyncFactor()
    l_setBeatPulseDelayMax(beat)
    l_setBeatPulseSpeedMult(2.00) -- Slows down the center going back to normal

    l_setSwapEnabled(true)
    l_setDarkenUnevenBackgroundChunk(false)
    l_setIncEnabled(false)

    if not u_inMenu() then
        maxChanges = u_rndInt(5, 12)
        lastRotationDir = getRandomDir()
        setDirection(u_rndInt(0, 6))
    end

    disableIncIfDMGreaterThan(1.5)

    t_wait(12 / getMusicDMSyncFactor())
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()
    addPattern(keys[index])
    index = index + 1

    if index - 1 == #keys then
        index = 1
        shuffle(keys)
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
    r, g, b, a = s_getMainColor()

    ArrayRemoveIf(floatingWalls, function(t, i, j)
        local v = t[i]
        if v.dead then
            cw_destroy(v.cwHandle)
            return true
        else
            cw_setVertexColor4Same(v.cwHandle, r, g, b, a)
            return false
        end
    end);

    for _, fw in ipairs(floatingWalls) do
        fw:move(mFrameTime)
    end

    if u_getDifficultyMult() <= 1.5 then
        timeAcc = timeAcc + mFrameTime
        if timeAcc >= 60 * 15 then
            timeAcc = 0
            changes = 0

            a_playSound("levelUp.ogg")

            beat = getBPMToBeatPulseDelay(180) / getMusicDMSyncFactor()
            t_wait(beat * 2)
            e_wait(beat * 2)
            t_eval([[a_playSound("increment.ogg")]])
            l_setSpeedMult(l_getSpeedMult() + l_getSpeedInc())
            if l_getSpeedMult() > l_getSpeedMax() then
                l_setSpeedMult(l_getSpeedMax())
            end
        end
    end

    if not achievementUnlocked and l_getLevelTime() > 45 and u_getDifficultyMult() >= 1 then
        steam_unlockAchievement("a36_bipolarity")
        achievementUnlocked = true
    end

    if not hardAchievementUnlocked and l_getLevelTime() > 30 and u_getDifficultyMult() > 1.5 then
        steam_unlockAchievement("a37_bipolarity_hard")
        hardAchievementUnlocked = true
    end
end

function swapStyle()
    if style == 0 then
        style = 1
        s_setStyle("bipolarity2")
        l_setRotationSpeed(rotSpeed * lastRotationDir * (u_getDifficultyMult() ^ 0.85))
        lastRotationDir = lastRotationDir * -1

        if rotSpeed < rotSpeedMax then
            rotSpeed = rotSpeed + 0.015
        end
    else
        style = 0
        s_setStyle("bipolarity")
        l_setRotationSpeed(0)
    end
end

function onCursorSwap()
    swapStyle()
    swapped = true
end
