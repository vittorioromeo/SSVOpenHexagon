-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "nextpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "evolutionpatterns.lua")

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    l_setSpeedMult(0.4)
    l_setSpeedInc(0.0)
    l_setRotationSpeed(0.0)
    l_setRotationSpeedMax(0.0)
    l_setRotationSpeedInc(0.0)
    l_setDelayMult(0.5)
    l_setDelayInc(0.0)
    l_setFastSpin(71.0)
    l_setSides(1024)
    l_setSidesMin(1024)
    l_setSidesMax(1024)
    l_setIncTime(10000)

    l_setWallSkewLeft(15)

    l_setPulseMin(75)
    l_setPulseMax(75)
    l_setPulseSpeed(0)
    l_setPulseSpeedR(0)
    l_setPulseDelayMax(6.8)

    l_setBeatPulseMax(20)
    l_setBeatPulseDelayMax(26.1)

    l_setSwapEnabled(true)
    l_setTutorialMode(true)
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
    e_messageAdd("remember, swap with spacebar!", 120)
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()
    hmcSimpleBarrageSNeigh(getRandomSide(), 3, 0)
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
