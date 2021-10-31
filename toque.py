import sympy as sym

seta_1 = sym.Symbol('seta_1')
seta_2 = sym.Symbol('seta_2')

# link_1 = sym.Symbol('link_1')
# link_2 = sym.Symbol('link_2')

link_1 = 10
link_2 = 10

m = 2
g = 9.8

# TODO! cos, sin 배열 생성

A = sym.Matrix([[sym.cos(seta_1+seta_2-180), sym.cos(seta_2)],
                [sym.sin(seta_1+seta_2-180), sym.sin(seta_2)]])
B = sym.Matrix(2, 1, [m*g/6, 0])
print('=======A Matrix=========\n', A)
print('=======B Matrix=========\n', B)

A_inv = A.inv()
print('=======A inverse=========\n', A_inv)

toque = A_inv*B
print('=======toque=========\n', toque)
