import math

SAM = 16
TORAD = 2*3.1428/SAM

class Wave:
    def __init__(self, name, gen, vector=None):
        self.name = name
        self.points = []
        if gen:
            for i in range(0,SAM):
                self.points.append( gen(i) )
        else:
            self.points = vector

    def header(self, f):
        f.write("const uint8_t {0}[];\r\n".format(self.name))
                
    def cpp(self, f):
        f.write("const uint8_t {0}[] = {{".format(self.name))
        for i in range(0,SAM):
            if i>0:
                f.write(",")
            f.write(" {0}".format(self.points[i]))
        f.write(" };\r\n")

#===========================================
def sq(i):
    if i>7:
        return 255
    else:
        return 0
#----------------------------------
def sin(i):
    return 127 + int( math.sin(i*TORAD)*127 + 0.5)
#----------------------------------
def sin2(i):
    return 127 + int( math.sin(i*TORAD)*90 + math.sin(i*TORAD*2)*40 + 0.5)
#----------------------------------

#----------------------------------

#===========================================
waves = [
    Wave("squareWave", sq),    
    Wave("sineWave", sin),
    Wave("sineWave2", sin2),
    Wave("triangle", None, [127, 158, 189, 220, 251, 220, 189, 158, 127, 96, 65, 34, 3, 34, 65, 96 ] )
]


with open("waves.h", "w") as of:
    of.write("""
#ifndef _WAVES_H
#define _WAVES_H
#include <stdint.h>
             
""")
    for w in waves:
        w.header(of)
    of.write("#endif")
    
with open("waves.cpp", "w") as of:
    of.write("""
#include "waves.h"
             
""")
    for w in waves:
        w.cpp(of)
