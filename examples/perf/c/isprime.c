#include <stdio.h>
#include <time.h>

int is_prime(int v)
{
	int i;
	for(i = 2; i < v / 2; ++i)
	{
		if(v % i == 0)
		{
			return 0;
		}
	}

	return 1;
}

int main(void)
{
	clock_t begin, end;
	double time_spent;
	int i, count;

	begin = clock();

	count = 0;
	for(i = 3; i <= 10000; ++i)
	{
		if(is_prime(i))
		{
			++count;
		}
	}

	printf("%d\n", count);

	end = clock();
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("time spent = %f\n", time_spent);
	return 0;
}
