from numericalunits import V, kohm, Mohm, kHz, pF
from math import pi

# Designing Gain and Offset in Thirty Seconds

# http://www.ti.com/lit/an/sloa097/sloa097.pdf

# 1. Introduction

Vref = 5 * V
Vout_fs = 5 * V
Vout_zs = 0 * V
Vin_fs = 2.5 * V
Vin_zs = -2.5 * V

# 2. Determining the Type of Function

m = Vout_fs = (Vout_fs - Vout_zs) / (Vin_fs - Vin_zs)
b = Vout_zs - m * Vin_zs

print("m =", m)
print("b =", b / V, "V")
print()


if m >= 0 and b >= 0:
    print("3. Positive m, positiv b")

    R1 = 100 * kohm
    R2 = Vref * R1 * m / b
    Rf = 100 * kohm
    Rg = R2 * Rf / (m * (R1 + R2) - R2)

    print("R1 = ", R1 / kohm, "k")
    print("R2 = ", R2 / kohm, "k")
    print("Rf = ", Rf / kohm, "k")
    print("Rg = ", Rg / kohm, "k")
    print()

    print("7.1 Filtering")

    f0 = 10 * kHz
    C0 = 1 / (2 * pi * R1 * f0)
    fo = 1 / (2 * pi * R2 * C0)

    print ("f0 = ", f0 / kHz, "kHz")
    print ("C0 = ", C0 / pF, "pF")
    print ("fo = ", fo / kHz, "kHz")
    print()
else:
    raise NotImplemented
