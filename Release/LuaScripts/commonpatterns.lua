execFile("common")

-- paltbarrage: spawns a series of caltbarrage
function paltbarrage(times, step)
	delay = getperfectdelay(thickness) * 4.6
	
	for i = 0, times do
		caltbarrage(i, step)
		wait(delay)
	end
	
	wait(delay)
end

-- pmirrorspiral: spawns a spiral of rwallex
function pmirrorspiral(times, extra)
	oldthickness = thickness
	thickness = getperfectthickness(thickness)
	delay = getperfectdelay(thickness)
	startside = getrandomside()
	loopdir = getrandomdir()	
	j = 0
	
	for i = 0, times do
		rwallex(startside + j, extra)
		j = j + loopdir
		wait(delay)
	end
	
	thickness = oldthickness
	
	wait(getperfectdelay(thickness) * 6.5)
end

-- pbarragespiral: spawns a spiral of cbarrage
function pbarragespiral(times, delaymult)
	delay = getperfectdelay(thickness) * 4.6 * delaymult
	startside = getrandomside()
	loopdir = getrandomdir()	
	j = 0
	
	for i = 0, times do
		cbarrage(startside + j)
		j = j + loopdir
		wait(delay)
	end
	
	wait(getperfectdelay(thickness) * 5.1)
end