#pragma once

#include <iostream>
#include "common.h"

class MessageBuffer {
public:
	static const bool Serialize(const MessageStruct& messageStruct, size_t& outSize, char* outData) {
		outSize = 0;
		outData[outSize++] = (char)messageStruct.type;

		size_t message_size = strlen(messageStruct.message);
		if (message_size + 5 >= MAX_BUF) {
			ZeroMemory(outData, MAX_BUF);
			outSize = 0;
			return false;
		}

		outData[outSize++] = message_size & 0xF;
		outData[outSize++] = (message_size >> 4) & 0xF;
		outData[outSize++] = (message_size >> 8) & 0xF;
		outData[outSize++] = (message_size >> 12) & 0xF;
		for (size_t i = 0; i != message_size; i++)
			outData[outSize++] = messageStruct.message[i];
		outData[outSize++] = '\0';
		return true;
	};

	static const MessageStruct Deserialize(const char* message) {
		MessageStruct result{};

		result.type = (MessageType)message[0];
		size_t message_size = 0;
		message_size += message[1];
		message_size += message[2] << 4;
		message_size += message[3] << 8;
		message_size += message[4] << 12;
		ZeroMemory(&result.message, MAX_BUF);

		for (size_t i = 0; i != message_size; i++) {
			result.message[i] = message[5 + i];
		}
		return result;
	};
};