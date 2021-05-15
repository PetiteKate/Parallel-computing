from numba import cuda, int32
import numpy as np
import math

TPB = 10 # max 32
N = TPB * 100 # matrix size max 5000

@cuda.jit
def fast_matmul(A, B, C):
    sA = cuda.shared.array(shape=(TPB, TPB), dtype=int32)
    sB = cuda.shared.array(shape=(TPB, TPB), dtype=int32)

    x, y = cuda.grid(2)

    tx = cuda.threadIdx.x
    ty = cuda.threadIdx.y
    bpg = cuda.gridDim.x

    tmp = 0.
    for i in range(bpg):
        sA[tx, ty] = 0
        sB[tx, ty] = 0
        if x < A.shape[0] and (ty+i*TPB) < A.shape[1]:
          sA[tx, ty] = A[x, ty + i * TPB]
        if y < B.shape[1] and (tx+i*TPB) < B.shape[0]:
          sB[tx, ty] = B[tx + i * TPB, y]
        cuda.syncthreads()
        for j in range(TPB):
            tmp += sA[tx, j] * sB[j, ty]
        cuda.syncthreads()
    if x < C.shape[0] and y < C.shape[1]:
        C[x, y] = tmp


cuda.detect() # информация о видеокарте

A = np.random.randint(-9, 9, size=(N, 1)).astype(np.int32) # создание А
B = np.random.randint(-9, 9, size=(1, N)).astype(np.int32)
C = np.zeros((N, N)).astype(np.int32)

d_a = cuda.to_device(A) # передача значений видеокарте
d_b = cuda.to_device(B)
d_c = cuda.to_device(C)

threadsperblock = (TPB, TPB) # количество потоков
blockspergrid_x = int(math.ceil(A.shape[0] / threadsperblock[1])) #разбиение на блоки
blockspergrid_y = int(math.ceil(B.shape[1] / threadsperblock[0]))
blockspergrid = (blockspergrid_x, blockspergrid_y)

fast_matmul[blockspergrid, threadsperblock](d_a, d_b, d_c)
res = d_c.copy_to_host()

print(res)