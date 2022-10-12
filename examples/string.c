int print_pyramid(int size)
{
	int i, j, num_stars;

	for(num_stars = 1, i = 1; i < size; i += 1, num_stars += 2)
	{
		for(j = size - 1; j >= 0; j -= 1)
		{
			if(j <= i)
			{
				break;
			}

			print_string(" ");
		}

		for(j = 0; j < num_stars; j += 1)
		{
			print_string("*");
		}

		print_string("\n");
	}
}

int main()
{
	print_pyramid(10);
}