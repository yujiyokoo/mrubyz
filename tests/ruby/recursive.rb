def recursive(x, y, z)
  return y if x == 0

  recursive(x - 1, y + z, z)
end

return recursive(3, 2, 4)
