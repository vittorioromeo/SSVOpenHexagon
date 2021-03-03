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

step0 = false
step1 = false
step2 = false
step25 = false
step3 = false
step35 = false
step4 = false
step5 = false
step6 = false

step0_trRotateLeft = "nope"
step0_trRotateRight = "nope"

step0_completionTime = 0
step3_completionTime = 0

step1_trFocus = "nope"

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
	l_setSpeedMult(1.00)
	l_setSpeedInc(0.20)
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
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()
	if not step35 then
		return
	end

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

-- onInput is a hardcoded function invoked when the player executes input
function onInput(mFrameTime, mMovement, mFocus, mSwap)
	if step1 then
		if mMovement == -1 then
			step0_trRotateLeft = "yes!"
		end

		if mMovement == 1 then
			step0_trRotateRight = "yes!"
		end
	end

	if step25 then
		if mFocus then
			step1_trFocus = "yes!"
		end
	end
end

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
	if not step0 then
		step0 = true

		e_messageAddImportant("welcome to open hexagon 2!", 120)
		e_messageAddImportant("let's learn about the controls", 120)
		e_messageAddImportant("use left/right to rotate around the center", 180)
		e_messageAddImportant("try it now!", 120)
		e_stopTimeS(9)
	end

	if not step1 and l_getLevelTime() > 0.2 then
		step1 = true

		l_addTracked("step0_trRotateLeft", "rotated left")
		l_addTracked("step0_trRotateRight", "rotated right")
	end

	if step1 and not step2 and step0_trRotateLeft == "yes!" and step0_trRotateRight == "yes!" then
		step2 = true
		step0_completionTime = l_getLevelTime()

		e_messageAddImportant("well done!", 120)
		e_messageAddImportant("you can slow down by focusing", 120)
		e_messageAddImportant("by default, use left shift", 120)
		e_messageAddImportant("try it now!", 120)
		e_stopTimeS(8)
	end

	if not step25 and step2 and l_getLevelTime() > step0_completionTime + 0.2 then
		step25 = true

		l_addTracked("step1_trFocus", "focused")
	end

	if step25 and not step3 and step1_trFocus == "yes!" then
		step3 = true
		step3_completionTime = l_getLevelTime()

		e_stopTimeS(7)
		e_messageAddImportant("great!", 120)
		e_messageAddImportant("the goal of open hexagon is to survive", 180)
		e_messageAddImportant("avoid the walls!", 120)
	end

	if not step35 and step3 and l_getLevelTime() > step3_completionTime + 0.2 then
		step35 = true

		l_clearTracked()
		l_resetTime()
	end

	if step35 and not step4 and l_getLevelTime() > 30 then
		step4 = true

		e_stopTimeS(5)

		if not challengeFailed then
			e_messageAddImportant("great job!", 110)
		else
			e_messageAddImportant("nice try...", 110)
		end

		e_messageAddImportant("after a while, things get harder", 120)
		e_messageAddImportant("get to 45 seconds to complete the tutorial!", 130)
	end

	if step4 and not step5 and l_getLevelTime() > 42 then
		step5 = true

		l_setIncEnabled(false);

		if not achievementUnlocked then
			steam_unlockAchievement("a0_babysteps")
			achievementUnlocked = true
		end

		if challengeFailed == false then
			a_playPackSound("fanfare.ogg")
			e_messageAddImportant("well done! you survived!", 120)
			e_messageAddImportant("now play some real levels!", 180)
		else
			a_playPackSound("failure.ogg")
			e_messageAddImportant("nice try, but not quite...", 120)
			e_messageAddImportant("try again without hitting any wall!", 220)
		end
	end

	if step5 and not step6 and l_getLevelTime() > 45 then
		step6 = true
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
