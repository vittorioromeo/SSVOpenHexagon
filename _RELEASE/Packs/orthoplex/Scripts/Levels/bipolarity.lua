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

        setDirection(direction + l_getSides() / 2)

        if dm > 1.5 then
            dm = 1.5
        end

        if dm < 1 then
            dm = 2
        end

        t_wait(10 * (dm ^ 1.2))
        t_eval([[mkSwapWall(]] .. oldDirection + l_getSides() / 2 .. [[)]])
        t_wait(10 * (dm ^ 1.2))
    else
        cBarrage(direction)

        if changes ~= maxChanges - 1 then
            setDirection(direction + getRandomDir())
        end

        local delay = getPerfectDelayDM(THICKNESS) * 5.6
        t_wait(delay)
    end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 0, 0, 0, 0, 0, 0, 3, 4, 5, 5 }
shuffle(keys)
index = 0
achievementUnlocked = false

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    l_setSpeedMult(7.00)
    l_setSpeedInc(0.125)
    l_setSpeedMax(5)
    l_setRotationSpeed(0.0)
    l_setRotationSpeedMax(1)
    l_setRotationSpeedInc(0.04)

    l_setDelayMult(1.85)

    l_setDelayInc(0)
    l_setFastSpin(0.0)
    l_setSides(6)
    l_setSidesMin(6)
    l_setSidesMax(6)
    l_setIncTime(15)

    l_setPulseMin(75)
    l_setPulseMax(125)
    l_setPulseSpeed(6.4)
    l_setPulseSpeedR(6.4)
    l_setPulseDelayMax(24.38)

    l_setBeatPulseInitialDelay(53 / 2)
    l_setBeatPulseMax(35)
    l_setBeatPulseDelayMax(20) -- BPM is 180
    l_setBeatPulseSpeedMult(2.00) -- Slows down the center going back to normal

    l_setSwapEnabled(true)
    l_setDarkenUnevenBackgroundChunk(false)
    l_setIncEnabled(false)

    if not u_inMenu() then
        maxChanges = u_rndInt(5, 12)
        lastRotationDir = getRandomDir()
        setDirection(u_rndInt(0, 6))
    end
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
end

function onCursorSwap()
    if style == 0 then
        style = 1
        s_setStyle("bipolarity2")
        l_setRotationSpeed(0.25 * lastRotationDir * (u_getDifficultyMult() ^ 0.85))
        lastRotationDir = lastRotationDir * -1
    else
        style = 0
        s_setStyle("bipolarity")
        l_setRotationSpeed(0)
    end
end
