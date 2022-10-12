# nano-c
Very small C interpreter

Modified version works on an Arduino Uno (only 2048 bytes RAM).

## Features

* if-statement
* for-, while-, do-while- loops
* break and continue
* functions and recursion
* local and global variables
* relatively helpful error messages
* int type (can be used as pointers to a global data area)
* generates bytecode that is then run by the interpreter

* user-defined native functions [NEW!]
* chars and strings [NEW!]
* 8, 16, 32 bit arrays [NEW!]

## Example code it can run

```C
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

