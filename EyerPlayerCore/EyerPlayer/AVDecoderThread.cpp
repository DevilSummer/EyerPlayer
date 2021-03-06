#include "EyerPlayerThread.hpp"
#include "EventTag.hpp"

namespace Eyer {
    AVDecoderThread::AVDecoderThread(Eyer::EyerAVStream & _stream, AVFrameQueueManager * _frameQueueManager)
    {
        frameQueueManager = _frameQueueManager;
        stream = _stream;
    }

    AVDecoderThread::~AVDecoderThread()
    {

    }

    int AVDecoderThread::SendPacket(Eyer::EyerAVPacket * pkt)
    {
        cacheSize += pkt->GetSize();
        pktQueue.Push(pkt);

        return 0;
    }

    int AVDecoderThread::GetPacketCount()
    {
        return pktQueue.Size();
    }

    int AVDecoderThread::GetPacketSize()
    {
        if(cacheSize < 0){
            cacheSize = 0;
        }
        return cacheSize;
    }
}