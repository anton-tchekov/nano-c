int main()
{
	response_code(200);
	content_type("text/html");
	response_body("<h1>Hello from CGI</h1>");

	return 0;
}
