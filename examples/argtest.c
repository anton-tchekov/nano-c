int error400()
{
	response_code(400);
	response_body("<h1>400 Bad Request</h1><hr><p>Invalid parameter</p>");
}

int main()
{
	int arg_name_addr, arg_addr, number, square, square_str, output;

	content_type("text/html");

	if(args_count() != 1)
	{
		error400();
		return 0;
	}

	arg_name_addr = 0;
	arg_addr = 256;

	arg_name(0, arg_name_addr, 256);
	arg(0, arg_addr, 256);

	if(strcmp(arg_name_addr, "number") != 0)
	{
		error400();
		return 0;
	}

	number = atoi(arg_addr);
	square = number * number;
	square_str = 512;
	itoa(square, square_str, 10);

	output = 1024;
	strcpy(output, "<h1>The square of ");
	strcat(output, arg_addr);
	strcat(output, " is ");
	strcat(output, square_str);
	strcat(output, ".</h1>");

	response_code(200);
	response_body(output);
	return 0;
}
