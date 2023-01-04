#pragma once

#include <concurrent_vector.h>
#include <concurrent_unordered_set.h>
#include "channel.h"

class ChannelManager {
public:
	ChannelManager() {
		this->channelSet.clear();
		this->channeles.clear();
	};
	~ChannelManager() {
		this->channeles.clear();
	};

	bool MakeChannel(const std::string name, const size_t maxUser = 30) {
		if (this->channelSet.find(name) != this->channelSet.end()) {
			this->channelSet.insert(
				std::make_pair(
					name,
					this->channeles.push_back(
						std::make_unique<Channel>(name, maxUser))
				)
			);
			return true;
		}
		else {
			return false;
		}
	}
private:
	concurrency::concurrent_unordered_set<std::string, concurrency::concurrent_vector<std::unique_ptr<Channel>>::iterator> channelSet;
	concurrency::concurrent_vector<std::unique_ptr<Channel>> channeles;
};