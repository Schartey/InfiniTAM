#pragma once

#include "../ORUtils/ImageTypes.h"
#include "TCPServer.h"

#include <map>
#include <functional>


namespace ServerUtils
{
	typedef enum { MONO_8u, RGB_8u, MONO_16u, MONO_16s, RGBA_8u, FORMAT_UNKNOWN = -1 } FormatType;

	class PNMImage
	{
	public:
		bool binary;
		int sizeX;
		int sizeY;
		int maxValue;
		short* data;

	public:
		PNMImage();

		FormatType format;

		bool ConvertToORImage(ORUChar4Image* image);
		bool ConvertToORImage(ORShortImage* image);

		~PNMImage();
	};

	struct TransactionData {
		int dataReceived = 0;
		bool awaitingBinary = false;
		PNMImage* loadingColorImage;
		PNMImage* loadingDepthImage;
	};

	class TCPImageServer : public TCPServer
	{
	private:
		std::map<TCPSocket*, TransactionData*> transactionData;

	public:
		// Hook this function into Infinitam Engine
		std::function<void(ORUChar4Image*, ORShortImage*)> OnImageReceived;

	public:
        TCPImageServer(std::string host_, int port_);
        ~TCPImageServer() override;

        int Init() override;
		
        void ProcessMsg(TCPSocket* socket, char* msg, unsigned long length) override;
	};
}
