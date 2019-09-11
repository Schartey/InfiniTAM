#pragma once

#include "../ServerUtils/TCPImageServer.h"
#include "ImageSourceEngine.h"

#include <queue>

namespace InputSource {

	class ImageHostPathGenerator
	{
	private:
		std::string host;
		const int port;

	public:
		ImageHostPathGenerator(const char *host_, const int port_);
		std::string getHost() const;
		int getPort() const;
	};

	template <typename PathGenerator>
	class TCPImageSourceEngine : public BaseImageSourceEngine
	{
	private:
		ORUChar4Image *cached_rgb;
		ORShortImage *cached_depth;

		mutable std::queue<ORUChar4Image*> queuedRGBImages;
		mutable std::queue<ORShortImage*> queuedDepthImages;

		void loadIntoCache(void) const;

		mutable size_t cachedFrameNo;
		size_t currentFrameNo;
		mutable bool cacheIsValid;

		PathGenerator pathGenerator;

        ServerUtils::TCPImageServer* tcpImageServer;

	public:
        TCPImageSourceEngine(const char *calibFilename, const PathGenerator& pathGenerator_, size_t initialFrameNo = 0);
		~TCPImageSourceEngine();

		void OnImageReceived(ORUChar4Image* rgbImage, ORShortImage* depthImage);
		bool hasMoreImages(void) const;
		void getImages(ORUChar4Image* rgb, ORShortImage* rawDepth);
		Vector2i getDepthImageSize(void) const;
		Vector2i getRGBImageSize(void) const;
		bool IsRunning(void) const;
	};
}
