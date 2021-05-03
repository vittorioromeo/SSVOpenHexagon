u_execScript("common.lua")

function pAltMirrorSpiral(mTimes, mExtra)
    local oldThickness = THICKNESS
    THICKNESS = getPerfectThickness(THICKNESS)
    local delay = getPerfectDelay(THICKNESS)
    local startSide = getRandomSide()
    local loopDir = getRandomDir()
    for k = 1, #mTimes do
        for i = 1, mTimes[k] do
            rWallEx(startSide, mExtra)
            if (k % 2) == 0 then
                startSide = startSide + loopDir
            else
                startSide = startSide - loopDir
            end
            t_wait(delay)
        end
    end

    THICKNESS = oldThickness

    t_wait(getPerfectDelay(THICKNESS) * 6.5)
end

function pAltTunnel(mTimes,mFree)
    local oldThickness = THICKNESS
    local myThickness = getPerfectThickness(THICKNESS)
    local delay = getPerfectDelay(myThickness) * 5
    local startSide = getRandomSide()
    local loopDir = getRandomDir()

    THICKNESS = myThickness

    for i = 0, mTimes do
        if i < mTimes then
            w_wall(startSide, myThickness + 5 * l_getSpeedMult() * delay)
        end

        cBarrageN(startSide + loopDir,mFree)
        t_wait(delay)

        loopDir = loopDir * -1
    end

    THICKNESS = oldThickness
end

function cycle(mSides)
    local eArray = {}
    local j = getRandomSide()
    for i = 1, mSides do
        eArray[i] = (i + j) % mSides + 1
    end
    return eArray
end

function pLadder(mTimes,mArray,myThickness)

    local delay = getPerfectDelay(myThickness)

    local eArray = {}
    local l = 1
    local s = #mArray/l_getSides()
    local t = u_rndInt(0, 100)

    for i = 1, mTimes do
        local q = (i+t) % s + 1
        for k = 1, l_getSides() do
            if(mArray[(q-1)*l_getSides() + k] ~= 0) then
                eArray[l] = 1
            else
                eArray[l] = 0
            end
            l = l + 1
        end

        if i ~= mTimes then
            for j = 1, 3 do
                for k = 1,l_getSides() do
                    if(mArray[(q-1)*l_getSides() + k] == 2) then
                        eArray[l] = 1
                    else
                        eArray[l] = 0
                    end
                    l = l + 1
                end
            end
        end
    end

    patternizer(eArray,myThickness)
    t_wait(delay*2)
end

function patternizer(mArray,myThickness)
    local delay = getPerfectDelay(myThickness)
    local eArray = cycle(l_getSides())

    local j = math.floor((#mArray) / l_getSides())

    for i = 1, j do
        for k = 1, l_getSides() do
            if mArray[(i - 1)*l_getSides() + k] == 1 then
                w_wall(eArray[k], myThickness)
            end
        end
        t_wait(delay)
    end
end
