def extended_euclidean(a, b):
    if a == 0:
        return b, 0, 1
    gcd, x1, y1 = extended_euclidean(b % a, a)
    x = y1 - (b // a) * x1
    y = x1
    return gcd, x, y

def mod_inverse(a, m):
    gcd, x, _ = extended_euclidean(a, m)
    if gcd != 1:
        return None  # Modular inverse doesn't exist
    else:
        return x % m

# Constants
q0 = 5
twobt_min_1 = 2**127

# Calculate the modular inverse
m0 = mod_inverse(q0, twobt_min_1)
print("M0: ", m0)

p0 = int((q0 * m0 - 1) / twobt_min_1)
print("P0: ", p0)

p = int(q0 + p0)
print("P: ", p)

m = int((twobt_min_1 * p + 1) / q0)
print("M: ", m)


threshold_value = int(2**128 / 10 + 1)
print("Threshold Value: ", threshold_value)
