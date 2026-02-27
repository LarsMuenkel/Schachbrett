
import numpy as np

# Coordinates (Filx, Rank) -> [Base, Shoulder, Elbow, Wrist]
# File 0..7 (A..H), Rank 0..7 (1..8)
data = {}

# A-Column (File 0) - Complete
data[(0,0)] = [64, 137, 50, 90]   # A1
data[(0,1)] = [61, 113, 87, 80]   # A2
data[(0,2)] = [55, 100, 81, 116]  # A3
data[(0,3)] = [53, 94, 76, 142]   # A4
data[(0,4)] = [45, 81, 84, 151]   # A5
data[(0,5)] = [39, 71, 89, 161]   # A6
data[(0,6)] = [33, 75, 76, 180]   # A7
data[(0,7)] = [17, 62, 89, 180]   # A8

# B-Column (File 1) - Partial
data[(1,0)] = [70, 126, 58, 99]   # B1
data[(1,1)] = [69, 115, 65, 120]  # B2
data[(1,2)] = [65, 102, 68, 139]  # B3
data[(1,3)] = [60, 91, 73, 152]   # B4

# We need to fill (0..7, 0..7)

# Helper: Get interpolated values
full_grid = np.zeros((8, 8, 4), dtype=int)

# 1. Analyze Base Angle (Theta)
# Base depends primarily on File (Y) and secondarily on Rank (X).
# Theta = atan(Y/X). 
# A1 (Low X, Max Neg Y) -> 64
# A8 (High X, Max Neg Y) -> 17
# This confirms A is on the side, and increasing Rank (Distance) reduces angle magnitude FROM CENTER?
# No, if Center is 90.
# A1: 64 (Delta 26). A8: 17 (Delta 73).
# Wait, mechanically, A8 is further away.
# If robot is at (0,0). Board is at X=150.
# A1: x=167, y=-122. Angle atan(122/167) = 36 deg. 90-36 = 54.
# A8: x=412, y=-122. Angle atan(122/412) = 16 deg. 90-16 = 74.
# Theory says Angle should INCREASE (closer to 90) as we go further back.
# But Data says DECREASE (64 -> 17).
# This implies the servo "0" is actually the WIDE side (Left or Right), not aligned with Cartesian axes standardly.
# Or the pivot is weird.
# Let's simple fit a trend surface (Polynomial) to the known Base values.
# Poly order 2 for X and Y interaction?
bases = []
coords = []
for (f, r), vals in data.items():
    bases.append(vals[0])
    coords.append([f, r])

# Fit Base = c0 + c1*f + c2*r + c3*f*r + c4*f^2 + c5*r^2
# We have 12 points. 6 coeffs.
from sklearn.linear_model import LinearRegression
from sklearn.preprocessing import PolynomialFeatures

X = np.array(coords)
poly = PolynomialFeatures(degree=2)
X_poly = poly.fit_transform(X) # generates [1, f, r, f^2, f*r, r^2]

model_base = LinearRegression()
model_base.fit(X_poly, bases)

# 2. Analyze Shoulder, Elbow, Wrist
# These depend on Distance R.
# R depends on f and r.
# We CAN fit them blindly using poly regression too, assuming the surface is smooth.
# 12 points is enough for quadratic surface.

model_shoulder = LinearRegression()
model_shoulder.fit(X_poly, [v[1] for v in data.values()])

model_elbow = LinearRegression()
model_elbow.fit(X_poly, [v[2] for v in data.values()])

model_wrist = LinearRegression()
model_wrist.fit(X_poly, [v[3] for v in data.values()])

# Generate Grid
print("const RobotPose CALIBRATED_SQUARES[8][8] = {")
for f in range(8):
    print(f"  {{ // File {chr(65+f)}")
    for r in range(8):
        # Use known if exists, otherwise predict
        if (f,r) in data:
            vals = data[(f,r)]
            pred = vals
            comment = " // Known"
        else:
            # Predict
            features = poly.transform([[f, r]])
            b = int(round(model_base.predict(features)[0]))
            s = int(round(model_shoulder.predict(features)[0]))
            e = int(round(model_elbow.predict(features)[0]))
            w = int(round(model_wrist.predict(features)[0]))
            
            # Constrain
            b = max(0, min(180, b))
            s = max(0, min(180, s))
            e = max(0, min(180, e))
            w = max(0, min(180, w))
            pred = [b, s, e, w]
            comment = " // Pred"
            
        print(f"    {{{pred[0]}, {pred[1]}, {pred[2]}, {pred[3]}, 0}}", end="")
        if r < 7: print(",", end="")
        print(f"\t{comment} {chr(65+f)}{r+1}")
    print("  }", end="")
    if f < 7: print(",")
    print()
print("};")
