int is_prime(int v)
{
	int i;
	for(i = 2; i < v / 2; i += 1)
	{
		if(v % i == 0)
		{
			return 0;
		}
	}

	return 1;
}

int main()
{
	int i, count;
	count = 0;
	for(i = 3; i <= 10000; i += 1)
	{
		if(is_prime(i))
		{
			count += 1;
		}
	}

	print_number(count);
	print_string("\n");
	return 0;
}
