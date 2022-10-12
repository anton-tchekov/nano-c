int main()
{
	int i;
	int n;
	int array;

	n = 10;
	n += 5 * 5;
	print_number(n);


	array = 30;
	for(i = 0; i < 10; i+=1)
	{
		array\i8[2 * i + 0] = 'A';
		array\i8[2 * i + 1] = 'B';
	}

	for(i = 0; i < 20; i+=1)
	{
		print_number(array\i8[i]);
	}

	for(i = 0; i < 10; i+=1)
	{
		array\i16[2 * i + 0] = 777;
		array\i16[2 * i + 1] = 888;
	}

	array\i16[0] = 777;

	for(i = 0; i < 20; i+=1)
	{
		print_number(array\i16[i]);
	}
}
