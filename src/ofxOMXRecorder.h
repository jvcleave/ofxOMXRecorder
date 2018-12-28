#pragma once

#include "OMX_Maps.h"

class ofxOMXRecorderListener
{
public:
    virtual void onFrameRecorded()=0;
    virtual void onRecordStart()=0;
    virtual void onFileWritten(string filePath)=0;

    
};
class ofxOMXRecorderSettings
{
public:
    
    int width;
    int height;
    int outputWidth;
    int outputHeight;
    int colorFormat;
    int fps;
    float bitrateMegabytesPerSecond;
    int keyFrameInterval;
    bool enablePrettyFileName;
    
    ofxOMXRecorderSettings()
    {
        width = 1024;
        height = 1024;
        outputWidth = 1280;
        outputHeight = 720;
        colorFormat = GL_RGBA;
        fps = 25;
        bitrateMegabytesPerSecond = 2.0;
        keyFrameInterval = 5;
        enablePrettyFileName = true;
    };
};


class ofxOMXRecorder: public ofThread
{
public:

    ofxOMXRecorder();
    ~ofxOMXRecorder();
    ofxOMXRecorderListener* listener;
    void setup(ofxOMXRecorderSettings);

    void startRecording();
    void stopRecording();
    bool isRecording();
    
    string createFileName();
    void setKeyFrameInterval(int);
    
    int getFrameCounter()
    {
        return frameCounter;
    }
    
    void close();
    
    bool canTakeFrame;
    bool isOpen;
    void resetValues();
    //void createEncoder();
    void destroyEncoder();
    ofxOMXRecorderSettings settings;
    
    bool stopRequested;
    ofBuffer recordingFileBuffer;
    int frameCounter;
    
    bool startedRecording;
    bool finishedRecording;
    
    void writeFile();
    void writeBuffer();
    ofDirectory saveFolder;
    vector<unsigned char*> pixelBufferQueue;
    vector<unsigned char*> garbageQueue;

    int pixelDataSize;
    void addFrame(unsigned char* pixels);
    void threadedFunction();

    OMX_HANDLETYPE resizer;
    OMX_BUFFERHEADERTYPE* resizeInputBuffer;
    OMX_BUFFERHEADERTYPE* resizeOutputBuffer;

    
    void onResizerEmptyBuffer();
    void onResizerFillBuffer();
    void onEncoderEmptyBuffer();
    void onEncoderFillBuffer();
    static OMX_ERRORTYPE resizerEmptyBufferDone(OMX_HANDLETYPE resizer,
                                                OMX_PTR pAppData,
                                                OMX_BUFFERHEADERTYPE*)
    {
        ofxOMXRecorder* recorder = static_cast<ofxOMXRecorder*>(pAppData);
        recorder->onResizerEmptyBuffer();
        return OMX_ErrorNone;
    }
    
    static OMX_ERRORTYPE
    resizerFillBufferDone(OMX_HANDLETYPE, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE*)
    {
        ofLogNotice(__func__) << endl;
        ofxOMXRecorder* recorder = static_cast<ofxOMXRecorder*>(pAppData);
        recorder->onResizerFillBuffer();
        return OMX_ErrorNone;
    }
    
    static OMX_ERRORTYPE 
    resizerEventHandlerCallback(OMX_HANDLETYPE hComponent, 
                                                OMX_PTR pAppData, 
                                                OMX_EVENTTYPE event, 
                                                OMX_U32 nData1, OMX_U32 nData2, 
                                                OMX_PTR pEventData)
    {
        //ofLog() << "resizerEventHandlerCallback: " << DebugEventHandlerString(hComponent, event, nData1, nData2, pEventData);
        
        return OMX_ErrorNone;
        
    }
    OMX_HANDLETYPE encoder;
    OMX_BUFFERHEADERTYPE* inputBuffer;

    OMX_PARAM_PORTDEFINITIONTYPE enablePortBuffers(OMX_HANDLETYPE handle, OMX_BUFFERHEADERTYPE** targetBuffer, int portIndex);

    OMX_BUFFERHEADERTYPE* encoderInputBuffer;
    OMX_BUFFERHEADERTYPE* encoderOutputBuffer;
    
    bool bufferAvailable;

    static OMX_ERRORTYPE 
    encoderEventHandlerCallback(OMX_HANDLETYPE hComponent, 
                                OMX_PTR pAppData, 
                                OMX_EVENTTYPE event, 
                                OMX_U32 nData1, OMX_U32 nData2, 
                                OMX_PTR pEventData)
    {
        //ofLog() << "encoderEventHandlerCallback: " << DebugEventHandlerString(hComponent, event, nData1, nData2, pEventData);
        
        
        return OMX_ErrorNone;
        
    }
    static OMX_ERRORTYPE 
    encoderEmptyBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
    {
        ofxOMXRecorder* recorder = static_cast<ofxOMXRecorder*>(pAppData);
        recorder->onEncoderEmptyBuffer();
        return OMX_ErrorNone;
    }
    
    static OMX_ERRORTYPE
    encoderFillBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* encoderOutputBuffer)
    {    
        ofxOMXRecorder* recorder = static_cast<ofxOMXRecorder*>(pAppData);
        recorder->onEncoderFillBuffer();
        return OMX_ErrorNone;

    }
};
