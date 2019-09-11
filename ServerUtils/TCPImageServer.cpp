#include "TCPImageServer.h"

#include <sstream>

namespace ServerUtils
{
	PNMImage::PNMImage()
	{

	}

	bool PNMImage::ConvertToORImage(ORUChar4Image* image)
	{
		image->ChangeDims(Vector2i{ sizeX, sizeY });
		Vector4u* dataPtr = image->GetData(MEMORYDEVICE_CPU);

		char *data;
		if (format != RGBA_8u) data = new char[sizeX*sizeY * 3];
		else data = (char*)image->GetData(MEMORYDEVICE_CPU);

		if (binary)
		{
			// Fill data with raw data
			//if (!pnm_readdata_binary(f, xsize, ysize, RGB_8u, data)) { fclose(f); delete[] data; return false; }
		}
		else
		{
			// Fill data with raw data
			//if (!pnm_readdata_ascii(f, xsize, ysize, RGB_8u, data)) { fclose(f); delete[] data; return false; }
		}

		if (format != RGBA_8u)
		{
			for (int i = 0; i < image->noDims.x*image->noDims.y; i++)
			{
				dataPtr[i].x = data[i * 3 + 0]; dataPtr[i].y = data[i * 3 + 1];
				dataPtr[i].z = data[i * 3 + 2]; dataPtr[i].w = 255;
			}

			delete[] data;
		}
		return true;
	}

	bool PNMImage::ConvertToORImage(ORShortImage* image)
	{
		image->ChangeDims(Vector2i{ sizeX, sizeY });
		short *dataPtr = image->GetData(MEMORYDEVICE_CPU);

		if (binary) {
			// Fill data with raw data
			//if (!pnm_readdata_binary(f, xsize, ysize, RGB_8u, data)) { fclose(f); delete[] data; return false; }
            //memcpy(dataPtr, data, sizeof(short)*sizeX*sizeY);
            memcpy(dataPtr, data, 2);
		}
		else {
			// Fill data with raw data
			//if (!pnm_readdata_ascii(f, xsize, ysize, RGB_8u, data)) { fclose(f); delete[] data; return false; }
		}

		return true;
	}

	PNMImage::~PNMImage()
	{

	}

    TCPImageServer::TCPImageServer(std::string host_, int port_) : TCPServer(host_, port_)
    {

    }

    int TCPImageServer::Init()
	{
        return TCPServer::Init();
	}

    void TCPImageServer::ProcessMsg(TCPSocket* socket, char* msg, unsigned long length)
	{

        std::string pgmFormat = "";
        pgmFormat.push_back(msg[0]);
        pgmFormat.push_back(msg[1]);

        short pgmWidth = short((msg[2] << 8) | msg[3]);
        short pgmHeight = short((msg[4] << 8) | msg[5]);
        int pgmMax = ushort((msg[6] << 8) | msg[7]);

        PNMImage* colorImage = new PNMImage();
        PNMImage* depthImage = new PNMImage();
        colorImage->sizeX = pgmWidth;
        colorImage->sizeY = pgmHeight;
        colorImage->maxValue = 1;
        depthImage->sizeX = pgmWidth;
        depthImage->sizeY = pgmHeight;
        depthImage->maxValue = pgmMax;

        if(strncmp(pgmFormat.c_str(), "P5", 2))
        {
            colorImage->binary = true;
            depthImage->binary = true;
        }

        if (pgmMax < 0) depthImage->format = FormatType::FORMAT_UNKNOWN;
        else if (pgmMax <= (1 << 8))  depthImage->format = FormatType::MONO_8u;
        else if (pgmMax <= (1 << 15)) depthImage->format = MONO_16s;
        else if (pgmMax <= (1 << 16)) depthImage->format = MONO_16u;

        colorImage->data = new short[pgmWidth*pgmHeight];
        depthImage->data = new short[pgmWidth*pgmHeight];

        printf("Format: %s\n", pgmFormat.c_str());
        printf("Format: %d\n", pgmWidth);
        printf("Format: %d\n", pgmHeight);
        printf("Format: %d\n", pgmMax);

        char* imageData = new char[pgmWidth*pgmHeight*2];
        memcpy(imageData, &msg[8], pgmWidth*pgmHeight*2);

        int j = 0;
        for (unsigned long i = 8; i < pgmWidth*pgmHeight*2+8; i+=2)
        {
            unsigned char* msgchar = (unsigned char*)msg;
            unsigned char char1 = msgchar[i];
            unsigned char char2 = msgchar[i + 1];
            short msgShort = ((short)char1 << 8) | char2;
            unsigned short msguShort = ((unsigned short)char1 << 8) | char2;
            //std::cout << msgShort << std::endl;
            depthImage->data[(i-8)/2] = msgShort;

            // Fake color image
            colorImage->data[(i-8)/2] = 0;
            i += 2;
        }

        if (OnImageReceived != nullptr)
        {
            ORUChar4Image *temp_color = new ORUChar4Image(Vector2i{ 0, 0 }, true, false, false);
            ORShortImage *temp_depth = new ORShortImage(Vector2i{ 0, 0 }, true, false, false);

            colorImage->ConvertToORImage(temp_color);
            depthImage->ConvertToORImage(temp_depth);
            OnImageReceived(temp_color, temp_depth);
        }
        Send(socket->socket, "Image Received");

        return;
	}

    TCPImageServer::~TCPImageServer()
    {

    }
}
