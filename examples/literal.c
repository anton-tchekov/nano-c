int a()
{
	print_number(0);
	return 1;
}

int b()
{
	print_number(2);
	return 1;
}

int main()
{
	int n;
	int v;
	n = 'A';
	v = print_number(n);

	if(a() && b())
	{
		print_number(3);
	}
}
