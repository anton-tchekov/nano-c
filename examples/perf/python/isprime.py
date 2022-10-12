import time

t0 = time.time()


def is_prime(v):
	for i in range(2, int(v / 2)):
		if v % i == 0:
			return False
	return True


count = 0
for i in range(3, 10000):
	if is_prime(i):
		count += 1

print(count)

t1 = time.time()

total = t1-t0

print(total)
