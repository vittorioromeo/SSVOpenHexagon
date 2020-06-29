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

-- clamp: clamps a number between two values
function clamp(input, min_val, max_val)
	if input < min_val then
		input = min_val
	elseif input > max_val then
		input = max_val
	end
	return input
end