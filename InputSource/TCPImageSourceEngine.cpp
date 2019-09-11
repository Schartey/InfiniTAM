#include "TCPImageSourceEngine.h"

using namespace std::placeholders;

namespace InputSource {
	ImageHostPathGenerator::ImageHostPathGenerator(const char* host_, const int port_) : host(host_), port(port_) {}

	std::string ImageHostPathGenerator::getHost() const {
		return host;
	}

	int ImageHostPathGenerator::getPort() const {
		return port;
	}

	template <typename PathGenerator>
	TCPImageSourceEngine<PathGenerator>::TCPImageSourceEngine(const char *calibFilename, const PathGenerator& pathGenerator_, size_t initialFrameNo)
		: BaseImageSourceEngine(calibFilename),
		pathGenerator(pathGenerator_)
	{
		currentFrameNo = initialFrameNo;
		cachedFrameNo = -1;

		cached_rgb = new ORUChar4Image(true, false);
		cached_depth = new ORShortImage(true, false);
		cacheIsValid = false;

        tcpImageServer = new ServerUtils::TCPImageServer(pathGenerator_.getHost(), pathGenerator_.getPort());
        tcpImageServer->OnImageReceived = std::bind(&TCPImageSourceEngine::OnImageReceived, this, _1, _2);
        tcpImageServer->SetGreetingMessage("Hello Drone");
        tcpImageServer->Init();
	}

	template <typename PathGenerator>
	TCPImageSourceEngine<PathGenerator>::~TCPImageSourceEngine()
	{
        delete tcpImageServer;
		delete cached_rgb;
		delete cached_depth;
	}

	template <typename PathGenerator>
	void TCPImageSourceEngine<PathGenerator>::OnImageReceived(ORUChar4Image* rgbImage, ORShortImage* depthImage)
	{
		queuedRGBImages.push(rgbImage);
		queuedDepthImages.push(depthImage);
	}

	template <typename PathGenerator>
	void TCPImageSourceEngine<PathGenerator>::loadIntoCache(void) const
	{
        cacheIsValid = false;
		if (currentFrameNo == cachedFrameNo) return;

		if (queuedRGBImages.size() > 0 && queuedDepthImages.size() > 0)
		{
			ORUChar4Image* temp_rgb = queuedRGBImages.front();
			ORShortImage* temp_depth = queuedDepthImages.front();

			cached_rgb->Swap(*temp_rgb);
			cached_depth->Swap(*temp_depth);

			queuedRGBImages.pop();
			queuedDepthImages.pop();

			cachedFrameNo = currentFrameNo;
            cacheIsValid = true;
		}
	}

	template <typename PathGenerator>
	bool TCPImageSourceEngine<PathGenerator>::hasMoreImages(void) const
	{
		loadIntoCache();
		return cacheIsValid;
	}

	template <typename PathGenerator>
	void TCPImageSourceEngine<PathGenerator>::getImages(ORUChar4Image* rgb, ORShortImage* rawDepth)
	{
		loadIntoCache();
		rgb->SetFrom(cached_rgb, ORUtils::MemoryBlock<Vector4u>::CPU_TO_CPU);
		rawDepth->SetFrom(cached_depth, ORUtils::MemoryBlock<short>::CPU_TO_CPU);

		++currentFrameNo;
	}

	template <typename PathGenerator>
	Vector2i TCPImageSourceEngine<PathGenerator>::getDepthImageSize(void) const
	{
		loadIntoCache();
		return cached_depth->noDims;
	}

	template <typename PathGenerator>
	Vector2i TCPImageSourceEngine<PathGenerator>::getRGBImageSize(void) const
	{
		loadIntoCache();
		return cached_rgb->noDims;
	}

	template <typename PathGenerator>
	bool TCPImageSourceEngine<PathGenerator>::IsRunning(void) const
	{
        return true;
//		return tcpImageServer->IsRunning();
	}
}

template class InputSource::TCPImageSourceEngine<InputSource::ImageHostPathGenerator>;
