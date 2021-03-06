#include "EyerPlayerThread.hpp"
#include "EyerAV/EyerAV.hpp"
#include "PlayerEvent.hpp"
#include <QDebug>


#include <QAudioFormat>
#include <QAudioOutput>

namespace EyerPlayer {
    AudioPlayThread::AudioPlayThread(Eyer::EyerEventQueue * _eventQueue, AVFrameQueueManager * _queueManager)
    {
        eventQueue = _eventQueue;
        queueManager = _queueManager;
    }

    AudioPlayThread::~AudioPlayThread()
    {
        if(out != nullptr){
            delete out;
            out = nullptr;
        }
    }

    void AudioPlayThread::Run()
    {
        SetRunning();

        while(!stopFlag){
            Eyer::EyerTime::EyerSleep(1000);

            AVFrameQueue * playerAudioFrameQueue = nullptr;
            queueManager->GetQueue(EventTag::FRAME_QUEUE_PLAYER_AUDIO, &playerAudioFrameQueue);
            if(playerAudioFrameQueue == nullptr){
                continue;
            }

            Eyer::EyerAVFrame * f = nullptr;
            playerAudioFrameQueue->FrontPop(&f);
            if(f != nullptr){
                if(io == nullptr){
                    QAudioFormat fmt;
                    fmt.setSampleRate(f->GetSampleRate());
                    fmt.setSampleSize(f->GetPerSampleSize() * 8);
                    fmt.setChannelCount(f->GetChannels());
                    fmt.setCodec("audio/pcm");
                    fmt.setByteOrder(QAudioFormat::LittleEndian);

                    Eyer::EyerAVAudioDateType t = f->GetAudioDateType();
                    if(t == Eyer::EyerAVAudioDateType::UNSIGNEDINT){
                        fmt.setSampleType(QAudioFormat::UnSignedInt);
                    }
                    if(t == Eyer::EyerAVAudioDateType::SIGNEDINT){
                        fmt.setSampleType(QAudioFormat::SignedInt);
                    }
                    if(t == Eyer::EyerAVAudioDateType::FLOAT){
                        fmt.setSampleType(QAudioFormat::Float);
                    }

                    // f->GetInfo();

                    QAudioDeviceInfo info = QAudioDeviceInfo::defaultOutputDevice();
                    if (!info.isFormatSupported(fmt)) {
                        qDebug()<<"default format not supported try to use nearest";
                        // fmt = info.nearestFormat(fmt);
                    }

                    out = new QAudioOutput(fmt);
                    io = out->start();
                }


                int len = f->GetAudioPackedData(nullptr);
                unsigned char * data = (unsigned char *)malloc(len);
                f->GetAudioPackedData(data);

                int alreadyWrited = 0;
                while(alreadyWrited < len){
                    if(stopFlag){
                        break;
                    }
                    int ret = io->write((char *)data + alreadyWrited, len - alreadyWrited);
                    alreadyWrited += ret;
                }

                free(data);

                delete f;
            }
            else{
                // qDebug() << "Get NULL" << endl;
            }
        }

        SetStoping();
    }
}
