-- initialize random seed
math.randomseed(os.time())
math.random()
math.random()
math.random()

-- shuffle: shuffles an array
function shuffle(t)
  local n = #t
  
  while n >= 2 do
    local k = math.random(n) -- 1 <= k <= n
    t[n], t[k] = t[k], t[n]
    n = n - 1
  end
  
  return t
end