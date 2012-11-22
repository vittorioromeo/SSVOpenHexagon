-- set common variables
thickness = 40;

-- getsides: returns current sides
function getsides()
	return getSides()
end

-- getspeedmult: returns current speed multiplier
function getspeedmult()
	return getSpeedMult()
end

-- getdelaymult: returns current delay multiplier
function getdelaymult()
	return getDelayMult();
end

-- getrandomside: returns random side
function getrandomside()
	return math.random(0, getsides() - 1)
end

-- getrandomdir: returns either 1 or -1
function getrandomdir()
	if math.random(0, 100) > 50 then
		return 1
	end
	
	return -1
end

-- getperfectdelay: returns time to wait for two walls to be next to each other
function getperfectdelay(pthickness)
	return pthickness / (5 * getspeedmult()) + ((math.abs(6 - getsides())) * 1.25)
end

-- getperfectthickness: returns a good thickness value in relation to human reflexes
function getperfectthickness(pthickness)
	return pthickness * getspeedmult() * getdelaymult()
end

-- cwall: creates a wall with the common thickness
function cwall(side)
	wall(side, thickness)
end

-- owall: creates a wall opposite to the side passed
function owall(side)
	cwall(side + getsides() / 2)
end

-- rwall: union of cwall and owall (created 2 walls facing each other)
function rwall(side)
	cwall(side)
	owall(side)
end

-- cwallex: creates a wall with extra walls attached to it 
function cwallex(side, extra)
	cwall(side);

	loopdir = 1;
	
	if extra < 0 then 
		loopdir = -1
	end
	
	for i = 0, extra, loopdir do
		cwall(side + i)
	end
end

-- owallex: creates a wall with extra walls opposite to side
function owallex(side, extra)
	cwallex(side + getsides() / 2, extra)
end

-- rwallex: union of cwallex and owallex
function rwallex(side, extra)
	cwallex(side, extra)
	owallex(side, extra)
end

-- cbarragen: spawns a barrage of walls, with a free side plus neighbors
function cbarragen(side, neighbors)
	for i = neighbors, getsides() - 2 - neighbors, 1 do
		cwall(side + i + 1)
	end
end

-- cbarrage: spawns a barrage of walls, with a single free side
function cbarrage(side)
	cbarragen(side, 0)
end

-- cbarrageonlyn: spawns a barrage of wall, with only free neighbors
function cbarrageonlyn(side, neighbors)
	cwall(side)
	cbarragen(side, neighbors)
end

-- caltbarrage: spawns a barrage of alternate walls
function caltbarrage(side, step)
	for i = 0, getsides() / step, 1 do
		cwall(side + i * step)
	end
end