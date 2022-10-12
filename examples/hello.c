/* recursive implementation of fibonacci */
int fib(int n)
{
	if(n == 0)
	{
		return 0;
	}
	else if(n == 1)
	{
		return 1;
	}
	else
	{
		return fib(n - 1) + fib(n - 2);
	}
}

/* factorial */
int fact(int n)
{
	int res;
	res = 1;
	while(n)
	{
		res *= n;
		n -= 1;
	}

	return res;
}

int sort(int p, int count)
{
	quicksort(p, 0, count - 1);
}

int quicksort(int arr, int first, int last)
{
	int i, j, pivot, temp;
	if(first < last)
	{
		pivot = first;
		i = first;
		j = last;

		while(i < j)
		{
			while((arr[i] <= arr[pivot]) && (i < last))
			{
				i += 1;
			}

			while(arr[j] > arr[pivot])
			{
				j -= 1;
			}

			if(i < j)
			{
				temp = arr[i];
				arr[i] = arr[j];
				arr[j] = temp;
			}
		}

		temp = arr[pivot];
		arr[pivot] = arr[j];
		arr[j] = temp;
		quicksort(arr, first, j - 1);
		quicksort(arr, j + 1, last);
	}
}

int find(int p, int count, int elem)
{
	int i;

	for(i = 0; i < count; i += 1)
	{
		if(p[i] == elem)
		{
			return i;
		}
	}

	return -1;
}

int main()
{
	int buf;
	int i, n;
	buf = 0; // pointer

	print_string("fib(20) = ");
	print_number(fib(20)); // Should be 55
	print_string("\n");

	print_string("fact(9) = ");
	print_number(fact(9)); // Should be 362880
	print_string("\n");

	print_string("Array Length = ");
	n = get_number(); // Get arr of values

	// Get values (and test for loop)
	for(i = 0; i < n; i += 1)
	{
		print_string("Array[");
		print_number(i); // Get arr of values
		print_string("] = ");
		buf[i] = get_number();
	}

	print_string("----------\n");

	for(i = 0; i < n; i += 1)
	{
		print_string("Array[");
		print_number(i); // Get arr of values
		print_string("] = ");
		print_number(buf[i]);
		print_string("\n");
	}

	print_string("----------\n");

	/* if it contains 12 */
	if(find(buf, n, 12) >= 0)
	{
		print_string("Array contains 12!\n");
	}

	sort(buf, n);

	// Print sorted (and test while loop)
	i = 0;
	while(i < n)
	{
		print_string("Sorted[");
		print_number(i); // Get arr of values
		print_string("] = ");
		print_number(buf[i]);
		print_string("\n");
		i += 1;
	}
}

