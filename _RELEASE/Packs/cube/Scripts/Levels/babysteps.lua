-- include useful files
u_execScript("utils.lua")
u_execScript("common.lua")
u_execScript("commonpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
	if mKey == 0 then pBarrageSpiral(math.random(1, 2), 1, 1) 
	elseif mKey == 1 then pInverseBarrage(0)
	elseif mKey == 2 then pAltBarrage(math.random(1, 3), 2)
	end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 1, 2 }
keys = shuffle(keys)
index = 0

-- onInit is an hardcoded function that is called when the level is first loaded
function onInit()
	l_setSpeedMult(1.85)
	l_setSpeedInc(0.05)
	l_setRotationSpeed(0.04)
	l_setRotationSpeedMax(0.4)
	l_setRotationSpeedInc(0.04)
	l_setDelayMult(1.0)
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
	m_messageAddImportant("welcome to open hexagon 2", 130)
	m_messageAddImportant("use left/right to rotate", 130)
	m_messageAddImportant("avoid the walls!", 130)
	e_eventStopTimeS(6) e_eventWaitS(6)
	
	e_eventStopTimeS(3) e_eventWaitUntilS(12)
	m_messageAddImportant("great job!", 130)
	m_messageAddImportant("after a while, things get harder", 130)
	m_messageAddImportant("get to 45 seconds to win!", 130)

	e_eventWaitUntilS(42)
	m_messageAddImportant("well done!", 130)
	m_messageAddImportant("now play some real levels!", 138)

	e_eventWaitUntilS(45)
	u_eventKill()
end

-- onStep is an hardcoded function that is called when the level timeline is empty
-- onStep should contain your pattern spawning logic
function onStep()	
	addPattern(keys[index])
	index = index + 1
	
	if index - 1 == table.getn(keys) then
		index = 1
	end
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