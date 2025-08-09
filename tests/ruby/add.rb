# When optimisation by mrbc happens, expressions like 1 + 2 will be a single number
# When optimisation is not enabled, the `+` operator always results in sending `:+`
# Here, we assign integers to a and b so it will be the ADD instruction (optimisation must be on)
a = 2
b = 4
c = a + b
return c
