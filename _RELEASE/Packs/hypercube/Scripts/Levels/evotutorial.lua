-- include useful files
execScript("utils.lua")
execScript("common.lua")
execScript("commonpatterns.lua")
execScript("nextpatterns.lua")
execScript("evolutionpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
		if mKey == 0 then cBarrage(0)
	elseif mKey == 1 then hmcBarrageN(0, 0, 0, 0.05, -3.8, 2.7, true); wait(55)
	elseif mKey == 2 then hmcBarrageN(0, 0, 0, -0.05, -2.7, 3.8, true); wait(55)
	end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 1, 1, 2, 2 }
keys = shuffle(keys)
index = 0

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
	l_setSpeedMult(1.1)
	l_setSpeedInc(0.045)
	l_setRotationSpeed(0.1)
	l_setRotationSpeedMax(0.4)
	l_setRotationSpeedInc(0.045)
	l_setDelayMult(1.0)
	l_setDelayInc(0.0)
	l_setFastSpin(71.0)
	l_setSides(6)
	l_setSidesMin(5)
	l_setSidesMax(7)
	l_setIncTime(0)

	l_setWallSkewLeft(18)

	l_setPulseMin(64)
	l_setPulseMax(84)
	l_setPulseSpeed(1.05)
	l_setPulseSpeedR(1.35)
	l_setPulseDelayMax(7)

	l_setBeatPulseMax(15)
	l_setBeatPulseDelayMax(110)

	l_setSwapEnabled(true)
end

-- onLoad is an hardcoded function that is called when the level is started/restarted
function onLoad()
	tutorialMode()
	stopIncrement()

	messageImportantAdd("welcome to the evolution tutorial", 120)
	messageImportantAdd("today you'll be introduced to...", 120)
	messageImportantAdd("1. swapping!", 100)
	messageImportantAdd("2. curving walls!", 100)
	messageImportantAdd("", 120)
	messageImportantAdd("press space or middle mouse button\nto swap", 250)
	messageImportantAdd("it allows you to rotate 180 degrees!", 200)
	messageImportantAdd("", 120)

	messageImportantAdd("now: curving walls", 120)
	messageImportantAdd("they can be simple...", 120)
	messageImportantAdd("", 120 * 3 + 80)

	wait(135 * 8)
	hmcSimpleBarrage(1)
	wait(100)
	hmcSimpleBarrage(-1)
	wait(50)
	hmcSimpleBarrage(1)
	wait(100)
	hmcSimpleBarrage(-2.5)
	wait(80)
	hmcSimpleBarrage(2.5)
	wait(80)
	hmcSimpleBarrage(3)

	wait(50)
	messageImportantAdd("...in various patterns...", 130)
	messageImportantAdd("", 120 * 5 + 80)
	wait(130)

	hmcSimpleTwirl(5, 1, 0)
	wait(50)
	hmcSimpleTwirl(5, -2.5, 0.3)

	messageImportantAdd("...or can accellerate!", 130)
	messageImportantAdd("", 120 * 4 + 40)
	wait(130)

	hmcBarrage(0, 0.05, -1.5, 3, true)
	wait(80)
	hmcBarrage(0, -0.05, -3, 3, true)
	wait(100)
	hmcBarrage(0, 0.1, -2, 2, true)
	wait(100)
	hmcBarrage(0, 0.1, -3, 3, true)
	wait(200)

	messageImportantAdd("they can also do crazy stuff!", 130)
	messageImportantAdd("", 120 * 8 + 50)

	hmcSimpleCage(2.5, 1)
	wait(80)
	hmcSimpleCage(2.5, -1)
	wait(100)
	hmcSimpleCage(2.5, 1)
	hmcSimpleCage(2.5, 1)
	wait(100)
	hmcSimpleCage(2.5, 1)
	hmcSimpleCage(2.5, -1)
	wait(100)
	hmcSimpleSpinner(1)
	wait(100)
	hmcSimpleSpinner(-2)
	wait(100)
	hmcSimpleSpinner(3)
	wait(100)
	hmcSimpleCage(1.5, 1)
	hmcSimpleCage(2.5, 1)
	wait(100)
	hmcSimpleCage(1.5, 1)
	hmcSimpleCage(2.5, -1)
	wait(100)
	hmcSimpleSpinner(1)
	hmcSimpleSpinner(1.2)
	wait(100)
	hmcSimpleSpinner(1)
	hmcSimpleSpinner(-1.2)
	wait(700)

	messageImportantAdd("well done!", 130)
	messageImportantAdd("now play some real levels!", 138)

	kill()
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

-- continuous direction change (even if not on level increment)
dirChangeTime = 600

-- onUpdate is an hardcoded function that is called every frame
function onUpdate(mFrameTime)
	dirChangeTime = dirChangeTime - mFrameTime;
	if dirChangeTime < 0 then
		-- do not change direction while fast spinning
		if isFastSpinning() == false then
			l_setRotationSpeed(l_getRotationSpeed() * -1.0)
			dirChangeTime = 400
		end
	end 
end