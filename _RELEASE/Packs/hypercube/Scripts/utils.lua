-- [[ ENUMERATORS ]] --
-- These are enumerators. They translate integers into more English vocabulary, making them much
-- easier to memorize over pure integers.
-- Use an enumator such as "EnumeratorName.VALUE" (e.g. KeyCode.LEFT)

-- An enumerator that defines basic movement values for onInput
local Movement = {
	LEFT = -1,
	NONE = 0,
	RIGHT = 1
};

-- An enumerator that defines all of the mouse buttons known by the SFML Mouse Button enumerator.
local MouseButton = {
	LEFT = 0,
	RIGHT = 1,
	MIDDLE = 2,
	XBUTTON_ONE = 3,
	XBUTTON_TWO = 4,
	BUTTON_COUNT = 5
};

-- An enumerator that defines all of the keycodes that is known by the SFML Key enumerator. 
-- Taken from Spyro Oshisaure's Keyboard Helper. Modified by Synth Morxemplum
local KeyCode = {
	UNKNOWN = -1,
	-- Alphabet
	A = 0, B = 1, C = 2, D = 3, E = 4, F = 5, G = 6, H = 7, I = 8, J = 9, K = 10, L = 11, 
	M = 12, N = 13, O = 14, P = 15, Q = 16, R = 17, S = 18, T = 19, U = 20, V = 21, W = 22, 
	X = 23, Y = 24, Z = 25,

	-- Numbers
	ZERO = 26, ONE = 27, TWO = 28, THREE = 29, FOUR = 30, FIVE = 31, SIX = 32, SEVEN = 33, 
	EIGHT = 34, NINE = 35,

	ESCAPE = 36,
	LEFT_CONTROL = 37,
	LEFT_SHIFT = 38,
	LEFT_ALT = 39,
	LEFT_SYSTEM = 40,
	RIGHT_CONTROL = 41,
	RIGHT_SHIFT = 42,
	RIGHT_ALT = 43,
	RIGHT_SYSTEM = 44,
	MENU = 45,
	LEFT_BRACKET = 46,
	RIGHT_BRACKET = 47,
	SEMICOLON = 48,
	COMMA = 49,
	PERIOD = 50,
	QUOTE = 51,
	SLASH = 52,
	BACKSLASH = 53,
	TILDE = 54,
	EQUAL = 55,
	DASH = 56,
	SPACE = 57,
	RETURN = 58, ENTER = 58,   -- Using both names just in case
	BACKSPACE = 59,
	TAB = 60,
	PAGE_UP = 61,
	PAGE_DOWN = 62,
	END = 63,
	HOME = 64,
	INSERT = 65,
	DELETE = 66,
	ADD = 67,
	SUBTRACT = 68,
	MULTIPLY = 69,
	DIVIDE = 70,

	-- Arrow keys
	LEFT = 71, RIGHT = 72, UP = 73, DOWN = 74,

	-- Numpad
	NUMPAD_ZERO = 75, NUMPAD_ONE = 76, NUMPAD_TWO = 77, NUMPAD_THREE = 78, 
	NUMPAD_FOUR = 79, NUMPAD_FIVE = 80, NUMPAD_SIX = 81, NUMPAD_SEVEN = 82, 
	NUMPAD_EIGHT = 83, NUMPAD_NINE = 84,

	-- Function keys
	F1 = 85, F2 = 86, F3 = 87, F4 = 88, F5 = 89, F6 = 90, F7 = 91, F8 = 92, F9 = 93, 
	F10 = 94, F11 = 95, F12 = 96, F13 = 97, F14 = 98, F15 = 99,

	PAUSE = 100,
	KEY_COUNT = 101
}

-- [[ UTILITY FUNCTIONS ]] --
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