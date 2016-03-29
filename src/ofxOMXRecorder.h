#pragma once

#include "OMX_Maps.h"


class ofxOMXRecorder
{
public:
    
 

    ofxOMXRecorder();
    ~ofxOMXRecorder();

    void setup(int width, int height, int colorFormat);
    void update(unsigned char* pixels);
    void startRecording();
    void startRecording(string filePath_);
    void stopRecording();
    bool isRecording();
    
    int numMBps;
    vector<ofFile>files;
private:
    
    int width;
    int height;
    int colorFormat;
    int pixelSize;
    bool stopRequested;
    ofBuffer recordingFileBuffer;
    string filePath;
    ofFbo* fbo;
    int frameCounter;
    void createFileName();
    bool startedRecording;
    bool videoFileWritten;
    bool finishedRecording;
    
    void writeFile();
    bool didWriteFile();

    void onPortSettingsChanged();
    void onEmptyBuffer();
    void onFillBuffer();
    
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