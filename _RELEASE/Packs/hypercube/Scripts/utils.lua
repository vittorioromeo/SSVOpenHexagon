--[[ UTILITY FUNCTIONS. ]]
-- Additional functions that help simplify certain calculations, including complex mathematical calculations
-- Ordered alphabetically

function clamp(input, min_val, max_val)
	--[[
		Clamps a number "input" between two values. The value can not go 
		below min_val and can not go above max_val.
	]]
	if input < min_val then
		input = min_val
	elseif input > max_val then
		input = max_val
	end
	return input
end

function easeIn(x)
	--[[
	Takes a value x from 0 to 1 and returns the value on a quadratic ease curve
	that goes from 0 to 1 accelerating.
	]]
	x = clamp(x, 0, 1);
	return x ^ 2;
end

function easeInOut(x) 
	--[[
	Takes a value x from 0 to 1 and returns the value on a quadratic ease curve
	that goes from 0 to 1.
	]]
	x = clamp(x, 0, 1);
	if (x < 0.5) then
		return 2 * x * x;
	end
	return -1 + (4 - 2 * x) * x;
end

function easeOut(x)
	--[[
	Takes a value x from 0 to 1 and returns the value on a quadratic ease curve
	that goes from 0 to 1 decelerating.
	]]
	x = clamp(x, 0, 1);
	return x * (2 - x);
end

function getSign(number) 
	--[[
	Takes a number and returns it's sign value.
	
	Negatives return -1, positives return 1, and zero returns 0.
	]]
	if number < 0 then
		return -1;
	elseif number > 0 then
		return 1;
	end
	return 0;
end

-- A data structure that stores results for prime numbers as a way to "cache" results.
-- This helps make isPrime run faster by knocking out calculations that were already done.
local prime_memoization = {
	[0] = false,
	[1] = false,
	[2] = true
};
function isPrime(integer, divisor)
	--[[
	Determines if an integer is a prime number or not.
	]]
	-- Makes the divisor parameter optional. By default, it should start at 2.
	divisor = divisor or 2; 

	if (prime_memoization[integer] ~= nil) then
    	return prime_memoization[integer];
  	end
	
	if (integer % divisor == 0) then
    	prime_memoization[integer] = false;
		return false;
	elseif (divisor * divisor > integer) then
    	prime_memoization[integer] = true;
		return true;
	end
	-- Make a recursive call to try other factors.
	return isPrime(integer, divisor + 1); 
end

function lerp(initial, final, i) 
	--[[
	Linear interpolation function. Takes number value initial and returns a
	value that is (i * 100) percent to final value.
	
	i is a number value between 
	
	Thanks to Zly for showing me the "precise" method
	]]
	i = clamp(i, 0, 1);
	return (1 - i) * initial + i * final;
end

function randomFloat(minimum, maximum)
	--[[
	Returns a random value between minimum and maximum, but is a floating point
	number.
	]]
	return math.random() * (maximum - minimum) + minimum;
end

function roundFloat(floatingNumber)
	--[[
	Takes in a number value and follows true rounding rules.
	Decimals < .5 will round down, while >= .5 will round up.
	
	Returns the rounded number value
	]]
	return math.floor(floatingNumber + 0.5);
end

function shuffle(t)
	--[[
		"Shuffles" an array by swapping elements randomly across a table.
	]]
	local j
	for i = #t, 2, -1 do
		j = math.random(i)
		t[i], t[j] = t[j], t[i]
	end
	return t
end

function simplifyFloat(number, places) 
	--[[
	Takes in a number value and rounds the number to places decimal 
	places. Very useful to avoid precision errors.
	
	Returns a number value (usually with a floating decimal point)
	]]
	return roundFloat(number * (10 ^ places)) / (10 ^ places);
end