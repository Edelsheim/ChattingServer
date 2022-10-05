#pragma once

#include <iostream>
#include <memory>
#include "src/server.h"

int main(void) {
	std::unique_ptr<Server> server = std::make_unique<Server>(3501);
	server->run();

	delete server.release();

	return 0;
}