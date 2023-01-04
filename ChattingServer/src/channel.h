#pragma once

#include <iostream>
#include <set>
#include <mutex>
#include "session.h"

class Channel {
public:
	Channel(const std::string name, const size_t max = 30) : channelName(name), maxClient(max) {
		this->currentClient = 0;
		this->clients.clear();
	};
	~Channel() {};

	const bool joinChannel(const Session& session) {
		std::lock_guard<std::mutex> guard(this->joinMt);

		if (this->currentClient >= this->maxClient)
			return false;

		this->currentClient++;
		this->clients.insert(session);
		return true;
	}

	const bool leaveChannel(const Session& session) {
		std::lock_guard<std::mutex> guard(this->leaveMt);

		if (this->currentClient <= 0)
			return false;
		
		this->currentClient--;
		this->clients.erase(session);
		return true;
	}

	const std::string getChannelName() const { return this->channelName; }
	const size_t getMaxClient() const { return this->maxClient; }
	const size_t getCurrentClient() const { return this->currentClient; }
private:
	std::string channelName;
	size_t maxClient;
	std::set<Session> clients;
	size_t currentClient;
	std::mutex joinMt;
	std::mutex leaveMt;
};