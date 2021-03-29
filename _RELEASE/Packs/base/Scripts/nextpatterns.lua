u_execScript("common.lua")
u_execScript("commonpatterns.lua")
u_execScript("utils.lua")
u_execScript("alternativepatterns.lua")

function wallSAdj(mSide, mAdj) w_wallAdj(mSide, THICKNESS, mAdj) end
function wallSAcc(mSide, mAdj, mAcc, mMinSpd, mMaxSpd) w_wallAcc(mSide, THICKNESS, mAdj, mAcc * (u_getDifficultyMult()), mMinSpd, mMaxSpd) end

function pTrapBarrage(mSide)
    local delay = getPerfectDelayDM(THICKNESS) * 3.7

    cBarrage(mSide)
    t_wait(delay * 3)
    wallSAdj(mSide, 1.9)

    t_wait(delay * 2.5)
end

function pTrapBarrageDouble(mSide)
    local delay = getPerfectDelayDM(THICKNESS) * 3.7
    local side2 = mSide + getHalfSides();

    for i = 0, l_getSides() - 1 do
        local currentSide = mSide + i
        if((currentSide ~= mSide) and (currentSide ~= side2)) then cWall(currentSide) end
    end

    t_wait(delay * 3)
    wallSAdj(mSide, 1.9)
    wallSAdj(side2, 1.9)

    t_wait(delay * 2.5)
end

function pTrapBarrageInverse(mSide)
    local delay = getPerfectDelayDM(THICKNESS) * 3.7

    cWall(mSide)
    t_wait(delay * 3)

    for i = 0, l_getSides() - 1 do
        local currentSide = mSide + i
        if(currentSide ~= mSide) then wallSAdj(currentSide, 1.9) end
    end

    t_wait(delay * 2.5)
end

function pTrapBarrageAlt(mSide)
    local delay = getPerfectDelayDM(THICKNESS) * 3.7

    for i = 0, l_getSides() - 1 do
        local currentSide = mSide + i
        if(currentSide % 2 ~= 0) then cWall(currentSide) end
    end

    t_wait(delay * 3)

    for i = 0, l_getSides() - 1 do
        local currentSide = mSide + i
        if(currentSide % 2 == 0) then wallSAdj(currentSide, 1.9) end
    end

    t_wait(delay * 2.5)
end

function pTrapSpiral(mSide)
    local delay = getPerfectDelayDM(THICKNESS) * 3.7
    local loopDir = getRandomDir()

    if(l_getSides() < 6) then delay = delay + 4 end

    for i = 0, l_getSides() + getHalfSides() do
        local currentSide = (mSide + i) * loopDir
        for j = 0, getHalfSides() do wallSAdj(currentSide + j, 1.2 + (i / 7.9)) end
        t_wait((delay * 0.75) - (i * 0.45) + 3)
    end

    t_wait(delay * 2.5)
end

function pRCBarrage()
    local currentSides = l_getSides()
    local delay = getPerfectDelayDM(THICKNESS) * 3.7
    local startSide = u_rndInt(0, 10)

    for i = 0, currentSides - 2 do
        local currentSide = startSide + i
        cWall(currentSide)
    end
    t_wait(delay * 2.5)
end

function pRCBarrageDouble()
    local currentSides = l_getSides()
    local delay = getPerfectDelayDM(THICKNESS) * 3.7
    local startSide = u_rndInt(0, 10)

    for i = 0, currentSides - 2 do
        local currentSide = startSide + i
        local holeSide = startSide + i + (currentSides / 2)
        if(i ~= holeSide) then cWall(currentSide) end
    end
    t_wait(delay * 2.5)
end

function pRCBarrageSpin()
    local currentSides = l_getSides()
    local delay = getPerfectDelayDM(THICKNESS) * 3.7
    local startSide = u_rndInt(0, 10)
    local loopDir = getRandomDir()

    for j = 0, 2 do
        for i = 0, currentSides - 2 do
            local currentSide = startSide + i
            cWall(currentSide + (j * loopDir))
        end
        t_wait(delay + 1)
    end
    t_wait(delay * 2.5)
end

function pACBarrage()
    local currentSides = l_getSides()
    local delay = getPerfectDelayDM(THICKNESS) * 3.7
    local startSide = u_rndInt(0, 10)

    for i = 0, currentSides - 2 do
        local currentSide = startSide + i
        wallSAcc(currentSide, 9 + u_rndInt(0, 1), -1.1, 1, 12)
    end
    t_wait(delay * 2.5)
end

function pACBarrageMulti()
    local currentSides = l_getSides()
    local delay = getPerfectDelayDM(THICKNESS) * 3.7
    local startSide = u_rndInt(0, 10)

    for i = 0, currentSides - 2 do
        local currentSide = startSide + i
        wallSAcc(currentSide, 10, -1.09, 0.31, 10)
        wallSAcc(currentSide, 0, 0.05, 0, 4.0)
        wallSAcc(currentSide, 0, 0.09, 0, 4.0)
        wallSAcc(currentSide, 0, 0.12, 0, 4.0)
    end
    t_wait(delay * 8)
end

function pACBarrageMultiAltDir()
    local currentSides = l_getSides()
    local delay = getPerfectDelayDM(THICKNESS) * 4
    local mdiff = 1 + math.abs(1 - u_getDifficultyMult())
    local startSide = u_rndInt(0, 10)
    local loopDir = getRandomDir()

    for i = 0, currentSides + getHalfSides() do
        local currentSide = startSide + i * loopDir
        wallSAcc(currentSide, 10, -1.095, 0.40, 10)
        t_wait((delay / 2.21) * (mdiff * 1.29))
        wallSAcc(currentSide + (getHalfSides() * loopDir), 0, 0.128, 0, 1.4)
    end
    t_wait(delay * 8)
end
