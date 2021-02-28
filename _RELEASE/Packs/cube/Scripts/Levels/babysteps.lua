-- include useful files
u_execScript("utils.lua")
u_execScript("common.lua")
u_execScript("commonpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
	if mKey == 0 then pBarrageSpiral(math.random(1, 2), 1, 1)
	elseif mKey == 1 then pInverseBarrage(0)
	elseif mKey == 2 then pAltBarrage(math.random(1, 3), 2)
	elseif mKey == 3 then pSpiral(12, 0)
	end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 1, 2, 3 }
shuffle(keys)
index = 0
achievementUnlocked = false
challengeFailed = false

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
	l_setSpeedMult(1.00)
	l_setSpeedInc(0.30)
	l_setRotationSpeed(0.04)
	l_setRotationSpeedMax(0.4)
	l_setRotationSpeedInc(0.04)
	l_setDelayMult(1.3)
	l_setDelayInc(0.0)
	l_setFastSpin(0.0)
	l_setSides(6)
	l_setSidesMin(6)
	l_setSidesMax(6)
	l_setIncTime(15)
	l_setTutorialMode(true)
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
	e_messageAddImportant("welcome to open hexagon 2", 120)
	e_messageAddImportant("use left/right to rotate", 120)
	e_messageAddImportant("avoid the walls!", 110)
	e_stopTimeS(4)
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

step0 = false
step1 = false
step2 = false

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
	if not achievementUnlocked and l_getLevelTime() > 40 then
		steam_unlockAchievement("a0_babysteps")
		achievementUnlocked = true
	end

	if not step0 and l_getLevelTime() > 10 then
		step0 = true

		e_stopTimeS(5)

		if not challengeFailed then
			e_messageAddImportant("great job!", 110)
		else
			e_messageAddImportant("nice try...", 110)
		end

		e_messageAddImportant("after a while, things get harder", 130)
		e_messageAddImportant("get to 45 seconds to win!", 120)
	end

	if not step1 and l_getLevelTime() > 42 then
		step1 = true

		l_setIncEnabled(false);

		if challengeFailed == false then
			e_messageAddImportant("well done! you survived!", 130)
			e_messageAddImportant("now play some real levels!", 138)
		else
			e_messageAddImportant("nice try, but not quite...", 130)
			e_messageAddImportant("try again!", 120)
		end
	end

	if not step2 and l_getLevelTime() > 45 then
		step2 = true
		e_kill()
	end
end

-- onPreDeath is an hardcoded function that is called when the player is killed, even
-- in tutorial mode
function onPreDeath()
	if challengeFailed == false and l_getLevelTime() < 42 then
		challengeFailed = true
		e_messageAddImportant("whoops!", 60)
	end
end
