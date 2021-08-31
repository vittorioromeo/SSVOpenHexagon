-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "nextpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "evolutionpatterns.lua")

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    l_setSpeedMult(1.2)
    l_setSpeedInc(0.1)
    l_setRotationSpeed(5.0)
    l_setRotationSpeedMax(50.0)
    l_setRotationSpeedInc(5.0)
    l_setDelayMult(1.5)
    l_setDelayInc(0.0)
    l_setFastSpin(71.0)
    l_setSides(12)
    l_setSidesMin(9)
    l_setSidesMax(15)
    l_setIncTime(5)

    l_setWallSkewLeft(15)

    l_setPulseMin(25)
    l_setPulseMax(75)
    l_setPulseSpeed(0.5)
    l_setPulseSpeedR(1.3)
    l_setPulseDelayMax(6.8)

    l_setBeatPulseMax(30)
    l_setBeatPulseDelayMax(15.1)

    l_setSwapEnabled(true)
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()
    hmcSimpleBarrageSNeigh(getRandomSide(), 3, 4)
    t_wait(getPerfectDelay(THICKNESS) * 6)
end

-- onIncrement is an hardcoded function that is called when the level difficulty is incremented
function onIncrement()
end

-- onUnload is an hardcoded function that is called when the level is closed/restarted
function onUnload()
end

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
end
