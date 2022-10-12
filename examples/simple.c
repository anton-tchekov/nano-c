int a()
{
	return 42;
}

int main()
{
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

	print_number(a());
	print_number(111);
	print_string(hello_world);
}
