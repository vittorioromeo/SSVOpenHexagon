-- [[ CONSTANTS ]] --
-- These are constants. These are values that may be very useful to use to accomplish a specific task
-- These have no direct involvement with making patterns, hence they belong here and not in "common"

-- Defining some mathematical constants
math.tau = math.pi * 2
math.e = math.exp(1)
math.phi = (1 + 5 ^ 0.5) / 2
SQRT_TWO = math.sqrt(2)
SQRT_THREE = math.sqrt(3)

-- Open Hexagon players, if certain things are causing issues for you please change this value
-- to the refresh rate of your monitor.
REFRESH_RATE = 60
FPS = 60

-- Taken from Omegasphere
-- These are constants to use when you want the rotation speed to match the horizontal speed of
-- curving walls.
CURVE_ROTATION_MULTIPLIER = 10.471975
CURVE_ROTATION_FOCUS = 2.0436


-- [[ ENUMERATORS ]] --
-- These are enumerators. They translate integers into more English vocabulary, making them much
-- easier to memorize over pure integers.
-- Use an enumator such as "EnumeratorName.VALUE" (e.g. KeyCode.LEFT)

-- An enumerator that defines basic movement values for onInput
Movement = {
    LEFT = -1,
    NONE = 0,
    RIGHT = 1
}

-- An enumerator that defines all of the mouse buttons known by the SFML Mouse Button enumerator.
MouseButton = {
    LEFT = 0,
    RIGHT = 1,
    MIDDLE = 2,
    XBUTTON_ONE = 3,
    XBUTTON_TWO = 4,
    BUTTON_COUNT = 5
}

-- An enumerator that defines all of the keycodes that is known by the SFML Key enumerator.
-- Taken from Spyro Oshisaure's Keyboard Helper. Modified by Synth Morxemplum
KeyCode = {
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

-- Checks to see if the current version of the game is equal to or greater than the version specified
-- Useful if you are developing a level in a beta branch or GitHub build.
function atRequiredVersion(mMinimumMajor, mMinimumMinor, mMinimumMicro)
    return u_getVersionMajor() >= mMinimumMajor and
           u_getVersionMinor() >= mMinimumMinor and
           u_getVersionMicro() >= mMinimumMicro
end

-- Converts Beats Per Minute (BPM) into period in Frames Per Beat (FPB)
function BPMtoFPB(bpm)
    return (FPS * 60)/bpm
end

-- Converts Beats Per Minute (BPM) into period in Seconds Per Beat (SPB)
function BPMtoSPB(bpm)
    return 60/bpm
end

-- Clamps a number "input" between two values. The value can not go
-- below min_val and can not go above max_val.
function clamp(input, min_val, max_val)
    if input < min_val then
        input = min_val
    elseif input > max_val then
        input = max_val
    end
    return input
end

-- Helper function for "fromHSV()", which translates from Hue and Saturation
-- to RGB values.
local function fromHS(h, s)
    -- Find a color from hue and saturation.
    h = (h % 360)/60
    local i, f, g, t
    i, f = math.modf(h)
    g = 1 - f -- For descending gradients
    t = 1 - s -- Minimum color intensity based on saturation
    f, g = s * f + t, s * g + t -- Apply saturation

        if i == 0 then return 1, f, t
    elseif i == 1 then return g, 1, t
    elseif i == 2 then return t, 1, f
    elseif i == 3 then return t, g, 1
    elseif i == 4 then return f, t, 1
    elseif i == 5 then return 1, t, g
    end
    return 1, 1, 1 -- Fallback
end

-- Converts HSV color values to RGB color values in a 0 - 255 range (alpha not included).
function fromHSV(h, s, v)
    -- Saturation and Value are optional parameters
    s = s or 1
    v = v or 1

    local r, g, b = fromHS(h, s)
    r = math.floor(r * v * 255)
    g = math.floor(g * v * 255)
    b = math.floor(b * v * 255)
    return r, g, b
end

-- Returns coordinates (x, y) adjusted with the level rotation using polar coordinate math.
function getAbsolutePosition(x, y)
    local r, a = (x ^ 2 + y ^ 2) ^ 0.5, math.atan(y, x)
    a = a + math.rad(l_getRotation())
    return r * math.cos(a), r * math.sin(a)
end

-- Takes a number and returns it's sign value.
-- Negatives return -1, positives return 1, and zero returns 0.
function getSign(number)
    if number < 0 then
        return -1
    elseif number > 0 then
        return 1
    end
    return 0
end

-- A data structure that stores results for prime numbers as a way to "cache" results.
-- This helps make isPrime run faster by knocking out calculations that were already done.
local prime_memoization = {
    [0] = false,
    [1] = false,
    [2] = true
}
-- Checks whether an integer is a prime number or not
function isPrime(integer)
    -- Check if our argument is an integer
    if (integer % 1 ~= 0) then
        return false
    end
    if (prime_memoization[integer]) then
        return prime_memoization[integer]
    end
    local divisor = 2
    while (divisor ^ 2 <= integer) do
        if (integer % divisor == 0) then
            prime_memoization[integer] = false
            return false
          end
          divisor = divisor + 1
    end
    prime_memoization[integer] = true
    return true
end

-- Inverse linear interpolation function. Takes two number values, initial
-- and final, and then a third number "value". Returns the percentage of
-- "value" between initial and final.
function inverseLerp(initial, final, value)
    return (value - initial) / (final - initial)
end

-- Linear interpolation function. Takes number value initial and returns a
-- value that is (i * 100) percent to final value.
-- i is a number value between 0 and 1
function lerp(initial, final, i)
    return (1 - i) * initial + i * final
end

-- Prints a table to the console. This includes all key value pairs, and recursively prints out any
-- tables of additional dimensions
-- Formatting of table contents are done with the "whitespace" character.
function printTable(luaTable, whitespace)
    whitespace = whitespace or ""

    if (type(luaTable) ~= "table") then return end

    for k, v in pairs(luaTable) do
        print(whitespace..tostring(k), v)
        if (type(v) == "table" and v ~= luaTable) then
            tablePrint(v, whitespace.."\t")
        end
    end
end

-- Returns a random value between minimum and maximum, but is a floating point
-- number.
function randomFloat(minimum, maximum)
    return u_rndReal() * (maximum - minimum) + minimum
end

-- Takes in a number value and follows true rounding rules.
-- Decimals < .5 will round down, while >= .5 will round up.
function roundFloat(floatingNumber)
    return math.floor(floatingNumber + 0.5)
end

-- "Shuffles" an array by swapping elements randomly across a table.
function shuffle(t)
    for i = #t, 3, -1 do
        local j = u_rndIntUpper(i - 1)
        t[i], t[j] = t[j], t[i]
    end
end

-- Takes in a number value and rounds the number to places decimal
-- places. Very useful to avoid floating point error.
function simplifyFloat(number, places)
    return roundFloat(number * (10 ^ places)) / (10 ^ places)
end

--[[ EASING FUNCTIONS ]]--
-- These are easing functions, which are a special subset of math functions that help with specific transitions.
-- These output values from 0 to 1, and work best when paired with linear interpolation (lerp).
-- All formulas are referenced from easings.net.

-- Sine
function easeInSine(x)
    x = clamp(x, 0, 1)
    return 1 - math.cos((x * math.pi) / 2)
end

function easeInOutSine(x)
    x = clamp(x, 0, 1)
    return -(math.cos(math.pi * x) - 1) / 2
end

function easeOutSine(x)
    x = clamp(x, 0, 1)
    return math.sin((x * math.pi) / 2)
end

-- Quadratic
function easeInQuad(x)
    x = clamp(x, 0, 1)
    return x ^ 2
end

function easeInOutQuad(x)
    x = clamp(x, 0, 1)
    if (x < 0.5) then
        return 2 * x * x
    end
    return 1 - (-2 * x + 2) ^ 2 / 2
end

function easeOutQuad(x)
    x = clamp(x, 0, 1)
    return 1 - (1 - x) * (1 - x)
end

-- Cubic
function easeInCubic(x)
    x = clamp(x, 0, 1)
    return x ^ 3
end

function easeInOutCubic(x)
    x = clamp(x, 0, 1)
    if (x < 0.5) then
        return 4 * x * x * x
    end
    return 1 - (-2 * x + 2) ^ 3 / 2
end

function easeOutCubic(x)
    x = clamp(x, 0, 1)
    return 1 - (1 - x) ^ 3
end

-- Quartic
function easeInQuart(x)
    x = clamp(x, 0, 1)
    return x ^ 4
end

function easeInOutQuart(x)
    x = clamp(x, 0, 1)
    if (x < 0.5) then
        return 8 * x * x * x * x
    end
    return 1 - (-2 * x + 2) ^ 4 / 2
end

function easeOutQuart(x)
    x = clamp(x, 0, 1)
    return 1 - (1 - x) ^ 4
end

-- Quintic
function easeInQuint(x)
    x = clamp(x, 0, 1)
    return x ^ 4
end

function easeInOutQuint(x)
    x = clamp(x, 0, 1)
    if (x < 0.5) then
        return 16 * x * x * x * x * x
    end
    return 1 - (-2 * x + 2) ^ 5 / 2
end

function easeOutQuint(x)
    x = clamp(x, 0, 1)
    return 1 - (1 - x) ^ 5
end

-- Exponential
function easeInExpo(x)
    x = clamp(x, 0, 1)
    if (x == 0) then
        return 0
    end
    return 2 ^ (10 * x - 10)
end

function easeInOutExpo(x)
    x = clamp(x, 0, 1)
    if (x == 0) then
        return 0
    elseif (x < 0.5) then
        return 2 ^ (20 * x - 10) / 2
    elseif (x == 1) then
        return 1
    end
    return (2 - 2 ^ (-20 * x + 10)) / 2
end

-- Easings.net has this formula WRONG. This is the actual formula right here.
function easeOutExpo(x)
    x = clamp(x, 0, 1)
    if (x == 1) then
        return 1
    end
    return -2 ^ (-10 * x) + 1
end

-- Circle
function easeInCirc(x)
    x = clamp(x, 0, 1)
    return 1 - math.sqrt(1 - (x^2))
end

function easeInOutCirc(x)
    x = clamp(x, 0, 1)
    if (x < 0.5) then
        return (1 - math.sqrt(1 - (2 * x) ^ 2)) / 2
    end
    return (math.sqrt(1 - (-2 * x + 2) ^ 2) + 1) / 2
end

function easeOutCirc(x)
    x = clamp(x, 0, 1)
    return math.sqrt(1 - (x - 1) ^ 2)
end

-- Back Functions
function easeInBack(x)
    x = clamp(x, 0, 1)
    local CONST_ONE = 1.70158
    local CONST_THREE = CONST_ONE + 1

    return CONST_THREE * x * x * x - CONST_ONE * x * x
end

function easeInOutBack(x)
    x = clamp(x, 0, 1)
    local CONST_ONE = 1.70158
    local CONST_TWO = CONST_ONE * 1.525

    if (x < 0.5) then
        return ((2 * x) ^ 2 * ((CONST_TWO + 1) * 2 * x - CONST_TWO)) / 2
    end
    return ((2 * x - 2) ^ 2 * ((CONST_TWO + 1) * (x * 2 - 2) + CONST_TWO) + 2) / 2
end

function easeOutBack(x)
    x = clamp(x, 0, 1)
    local CONST_ONE = 1.70158
    local CONST_THREE = CONST_ONE + 1

    return 1 + CONST_THREE * (x - 1) ^ 3 +
            CONST_ONE * (x - 1) ^ 2
end

-- Elastic
function easeInElastic(x)
    x = clamp(x, 0, 1)
    local CONST_FOUR = (2 * math.pi) / 3

    if (x == 0) then
        return 0
    elseif (x == 1) then
        return 1
    end
    return -(2 ^ (10 * x - 10)) * math.sin((x * 10 - 10.75) * CONST_FOUR)
end

function easeInOutElastic(x)
    x = clamp(x, 0, 1)
    local CONST_FIVE = (2 * math.pi) / 4.5

    if (x == 0) then
        return 0
    elseif (x == 1) then
        return 1
    elseif (x < 0.5) then
        return -(2 ^ (20 * x - 10)) * math.sin((20 * x - 11.125) * CONST_FIVE) / 2
    end
    return (2 ^ (-20 * x + 10) * math.sin((20 * x - 11.125) * CONST_FIVE)) / 2 + 1
end

function easeOutElastic(x)
    x = clamp(x, 0, 1)
    local CONST_FOUR = (2 * math.pi) / 3

    if (x == 0) then
        return 0
    elseif (x == 1) then
        return 1
    end
    return 2 ^ (-10 * x) * math.sin((x * 10 - 0.75) * CONST_FOUR) + 1
end

-- Bounce
-- Okay. Easings.net has this completely wrong here. Their implementation is garbage.
-- This is a much better implementation (Thanks Oshisaure)
function easeInBounce(x, bounceFactor)
    bounceFactor = bounceFactor or 1 -- Optional parameter
    x = clamp(x, 0, 1)
    if (x == 0) then
        return 0
    end
    return math.abs(math.cos(bounceFactor * (1 - 1/x) * math.pi) * math.sin(x * math.pi/2) ^ 2)
end

function easeInOutBounce(x)
    x = clamp(x, 0, 1)
    if (x < 0.5) then
        return easeInBounce(x)
    end
    return easeOutBounce(x)
end

function easeOutBounce(x, bounceFactor)
    x = clamp(x, 0, 1)
    if (x == 1) then
        return 1
    end
    return -easeInBounce(1 - x, bounceFactor) + 1
end

-- From: https://stackoverflow.com/questions/12394841/
function ArrayRemoveIf(t, fnRemove)
    local j, n = 1, #t

    for i=1,n do
        if (not fnRemove(t, i, j)) then
            -- Move i's kept value to j's position, if it's not already there.
            if (i ~= j) then
                t[j] = t[i]
                t[i] = nil
            end
            j = j + 1 -- Increment position of where we'll place the next kept value.
        else
            t[i] = nil
        end
    end

    return t
end

function getBPMToBeatPulseDelay(bpm)
    return 3600 / bpm
end

function getMusicDMSyncFactor()
    return u_getDifficultyMult() ^ 0.12
end
