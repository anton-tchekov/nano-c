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
	int res = 1;
	while(n)
	{
		res *= n;
		n -= 1;
	}

	return res;
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

int sort(int ptr, int count)
{
	quicksort(ptr, 0, count - 1);
}

int find(int ptr, int count, int elem)
{
	int i;
	for(i = 0; i < count; i += 1)
	{
		if(ptr[i] == elem)
		{
			return i;
		}
	}

	return -1;
}

int main()
{
	int buf = 0; // pointer
	int i, n;

	print(fib(10)); // Should be 55
	print(fact(9)); // Should be 362880

	n = get(); // Get arr of values

	// Get values (and test for loop)
	for(i = 0; i < n; i += 1)
	{
		buf[i] = get();
	}

	/* if it contains 12 */
	if(find(buf, n, 12) >= 0)
	{
		print(55555555);
	}

	sort(buf, n);

	// Print sorted (and test while loop)
	i = 0;
	while(i < n)
	{
		print(buf[i]);
		i += 1;
	}
}

