  lw 0 2 a
  lw 0 3 b
  lw 0 4 c
  lw 0 5 d
mul add 6 3 6   //5*4
  add 2 5 2
  jma 4 2 mul
  halt
a .fill 0
b .fill 5
c .fill 4
d .fill 1