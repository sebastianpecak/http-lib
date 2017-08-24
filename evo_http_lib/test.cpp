#include <HttpLib.hpp>
#include <Connection.hpp>
#include <Request.hpp>

int main() {
	// Create request.
	Http::Request request;
	request.SetMethod(Http::POST);
	request.SetVersion(Http::HTTP_11);
	request.SetSite("/lisener3des.php");
	request.SetProperty("Host", "195.78.239.101");
	request.SetProperty("Content-Length", "0");

	Http::Connection connection;
	connection.Open("195.78.239.101", 2080, false);
	request.Send(connection);
}