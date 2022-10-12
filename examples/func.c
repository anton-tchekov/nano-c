int func_a(int val)
{
	return 2 * func_b(val);
}

int func_b(int val)
{
	return 11 + val;
}

int mul(int a, int b)
{
	return a * b;
}

int add(int a, int b)
{
	return a + b;
}

int main()
{
	int dummy, a, b, c;
	int hello_world;

	hello_world = 0;

	hello_world\i8[0] = 'H';
	hello_world\i8[1] = 'e';
	hello_world\i8[2] = 'l';
	hello_world\i8[3] = 'l';
	hello_world\i8[4] = 'o';
	hello_world\i8[5] = ' ';
	hello_world\i8[6] = 'W';
	hello_world\i8[7] = 'o';
	hello_world\i8[8] = 'r';
	hello_world\i8[9] = 'l';
	hello_world\i8[10] = 'd';
	hello_world\i8[11] = '!';
	hello_world\i8[12] = '\n';
	hello_world\i8[13] = '\0';

	{
		int i;
		for(i = 0; i < 14; i += 1)
		{
			print_number(hello_world\i8[i]);
		}
	}

	c = add(mul(2 * 6, 5), mul(4, 3));
	print_number(c);

	print_string(hello_world);

	c = func_a(10);

	print_number(c);
}
