#include <iostream>
#include <memory>
#include "src/client.h"

int main(void) {
	std::unique_ptr<Client> client = std::make_unique<Client>();
	if (client->connect("127.0.0.1", 3501)) {

		while (true) {
			std::string send_data = "";
			printf("> ");
			std::cin >> send_data;
			client->send(send_data);
		}
	}

	delete client.release();

	return 0;
}