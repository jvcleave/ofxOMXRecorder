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
    int colorFormat;
    int fps;
    float bitrateMegabytesPerSecond;
    int keyFrameInterval;
    bool enablePrettyFileName;
    
    ofxOMXRecorderSettings()
    {
        width = 1280;
        height = 720;
        colorFormat = GL_RGBA;
        fps = 25;
        bitrateMegabytesPerSecond = 2.0;
        keyFrameInterval = 60;
        enablePrettyFileName = true;
    };
};


class ofxOMXRecorder
{
public:

    ofxOMXRecorder();
    ~ofxOMXRecorder();
    ofxOMXRecorderListener* listener;
    void setup(ofxOMXRecorderSettings);
    void update(unsigned char* pixels);

    void startRecording(string absoluteFilePath_="");
    void stopRecording();
    bool isRecording();
    
    vector<ofFile>recordings;
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
    void createEncoder();
    void destroyEncoder();
    ofxOMXRecorderSettings settings;
    
    int pixelSize;
    bool stopRequested;
    ofBuffer recordingFileBuffer;
    string absoluteFilePath;
    int frameCounter;
    
    bool startedRecording;
    bool finishedRecording;
    
    void writeFile();

    
    OMX_HANDLETYPE encoder;
    OMX_PARAM_PORTDEFINITIONTYPE enablePortBuffers(OMX_HANDLETYPE handle, OMX_BUFFERHEADERTYPE** targetBuffer, int portIndex);

    OMX_BUFFERHEADERTYPE* inputBuffer;
    OMX_BUFFERHEADERTYPE* outputBuffer;
    
    bool bufferAvailable;
    OMX_PARAM_PORTDEFINITIONTYPE inputPortDefinition;
    OMX_PARAM_PORTDEFINITIONTYPE outputPortDefinition;
    
    static OMX_ERRORTYPE 
    encoderEventHandlerCallback(OMX_HANDLETYPE, 
                                OMX_PTR, 
                                OMX_EVENTTYPE, 
                                OMX_U32, OMX_U32, 
                                OMX_PTR);
    static OMX_ERRORTYPE 
    encoderEmptyBufferDone(OMX_IN OMX_HANDLETYPE, 
                           OMX_IN OMX_PTR, 
                           OMX_IN OMX_BUFFERHEADERTYPE*);
    
    static OMX_ERRORTYPE
    encoderFillBufferDone(OMX_IN OMX_HANDLETYPE,
                          OMX_IN OMX_PTR,
                          OMX_IN OMX_BUFFERHEADERTYPE*);
};
