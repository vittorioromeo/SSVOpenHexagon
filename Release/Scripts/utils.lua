-- initialize random seed
math.randomseed(os.time())
math.random()
math.random()
math.random()

-- shuffle: shuffles an array
function shuffle(t)
	math.randomseed(os.time())
	local iterations = #t
	local j
	for i = iterations, 2, -1 do
			j = math.random(i)
			t[i], t[j] = t[j], t[i]
	end
	
	return t
end