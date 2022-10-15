-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "nextpatterns.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "evolutionpatterns.lua")

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
    l_setSpeedMult(2.7)
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
    l_setTutorialMode(true)
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
    ct0 = ct_create()
    ct1 = ct_create()
    ct2 = ct_create()

    ct_eval(ct0, [[u_log("ct0_A")]])
    ct_wait(ct0, 60)
    ct_eval(ct0, [[u_log("ct0_B")]])

    ct_eval(ct1, [[u_log("ct1_A")]])
    ct_wait(ct1, 60)
    ct_eval(ct1, [[u_log("ct1_B")]])

    ct_eval(ct2, [[u_log("ct2_A")]])
    ct_wait(ct2, 60)
    ct_eval(ct2, [[u_log("ct2_B")]])

    ct3 = ct_create()

    ct_eval(ct3, [[u_log("ct3_A")]])
    ct_wait(ct3, 20)
    ct_eval(ct3, [[u_log("ct3_AX")]])
    ct_wait(ct3, 20)
    ct_eval(ct3, [[u_log("ct3_AY")]])
    ct_wait(ct3, 20)
    ct_eval(ct3, [[u_log("ct3_B")]])
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()
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
