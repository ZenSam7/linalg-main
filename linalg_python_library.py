import linalg as la

a = la.Matrix([[1, 2],
               [3, 4]])
b = la.Matrix(2, 2)
b[0, 0] = 3
c = a * b + a
a += b
print(c, c.shape, a.det(), la.invert(a).T, c*la.invert(c), sep="\n")

try:
    la.invert(la.Matrix([[1, 2], [2, 4]]))
except la.MatrixIsSingular as e:
    print(e)