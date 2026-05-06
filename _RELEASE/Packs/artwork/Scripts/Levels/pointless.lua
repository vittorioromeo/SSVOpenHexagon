-- include useful files
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "common.lua")
u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "commonpatterns.lua")

function addPattern(mKey)
        if mKey == 0 then pAltBarrage(u_rndInt(3, 5), 2)
    elseif mKey == 1 then
        pMirrorSpiral(u_rndInt(2, 5), getHalfSides() - 3)
    elseif mKey == 2 then pBarrageSpiral(u_rndInt(0, 3), 1, 1)
    elseif mKey == 3 then pInverseBarrage(0)
    elseif mKey == 4 then
        pTunnel(u_rndInt(1, 3))
    elseif mKey == 5 then
        pSpiral(l_getSides() * u_rndInt(1, 2), 0)

        if l_getSpeedMult() >= 4.25 then
            t_wait(getPerfectDelayDM(THICKNESS) * 0.5)
        end
    end
end

keys = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 5, 5 }
shuffle(keys)
index = 0
achievementUnlocked = false
hardAchievementUnlocked = false
gradientShaderId = shdr_getShaderId("steamart.frag")

function onInit()
    THICKNESS = THICKNESS * 3.0

    l_setSpeedMult(0.35)

    l_setSpeedInc(0.125)
    l_setSpeedMax(4.75)
    l_setRotationSpeed(0.07)
    l_setRotationSpeedMax(0.9)
    l_setRotationSpeedInc(0.0)
    l_setDelayMult(1.0)
    l_setDelayInc(0)
    l_setFastSpin(0.0)
    l_setSides(6)
    l_setSidesMin(6)
    l_setSidesMax(6)
    l_setIncTime(1500000)

    l_setPulseMin(75)
    l_setPulseMax(91)
    l_setPulseSpeed(0.05)
    l_setPulseSpeedR(1)
    l_setPulseDelayMax(23.9)

    l_setBeatPulseMax(18)
    l_setBeatPulseDelayMax(1500000)
    l_setBeatPulseSpeedMult(0.38)

    shdr_setActiveFragmentShader(RenderStage.BACKGROUNDTRIS, gradientShaderId)
    shdr_setActiveFragmentShader(RenderStage.WALLQUADS3D, gradientShaderId)
    shdr_setActiveFragmentShader(RenderStage.PIVOTQUADS3D, gradientShaderId)
    shdr_setActiveFragmentShader(RenderStage.PLAYERTRIS3D, gradientShaderId)
    shdr_setActiveFragmentShader(RenderStage.WALLQUADS, gradientShaderId)
    shdr_setActiveFragmentShader(RenderStage.CAPTRIS, gradientShaderId)
    shdr_setActiveFragmentShader(RenderStage.PIVOTQUADS, gradientShaderId)
    shdr_setActiveFragmentShader(RenderStage.PLAYERTRIS, gradientShaderId)
end

function onLoad()
end

function onStep()
    addPattern(keys[index])
    index = index + 1

    if index - 1 == #keys then
        index = 1
        shuffle(keys)
    end
end

function onIncrement()
end

function onUnload()
end

function onUpdate(mFrameTime)
    shdr_setUniformFVec2(gradientShaderId, "u_resolution", u_getWidth(), u_getHeight());
    shdr_setUniformF(gradientShaderId, "u_time", l_getLevelTime())
    shdr_setUniformF(gradientShaderId, "u_rotation", math.rad(l_getRotation()))
end
