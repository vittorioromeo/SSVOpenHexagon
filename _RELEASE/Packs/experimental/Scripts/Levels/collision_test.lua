-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "nextpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "evolutionpatterns.lua")

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    l_setSpeedMult(5.0)
    l_setSpeedInc(0.0)
    l_setRotationSpeed(0.0)
    l_setRotationSpeedMax(0.0)
    l_setRotationSpeedInc(0.0)
    l_setDelayMult(1.35)
    l_setDelayInc(0.0)
    l_setFastSpin(71.0)
    l_setSides(6)
    l_setSidesMin(6)
    l_setSidesMax(6)
    l_setIncTime(10)

    l_setPulseMin(75)
    l_setPulseMax(75)
    l_setPulseSpeed(0)
    l_setPulseSpeedR(0)
    l_setPulseDelayMax(6.8)

    l_setBeatPulseMax(20)
    l_setBeatPulseDelayMax(26.1)

    l_addTracked("thickness", "Wall Thickness");

    l_setSwapEnabled(true)
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
end

thickness = 40;
-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()
    w_wall(0, thickness)
    w_wall(3, 80)
    t_wait(getPerfectDelay(THICKNESS));
    thickness = thickness + 1
    thickness = clamp(thickness, 40, 75);
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
