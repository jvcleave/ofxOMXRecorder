#pragma once
#include "OMX_Maps.h"

class ofxOMXImageDecoder
{
public:
    ofxOMXImageDecoder();
    ~ofxOMXImageDecoder();
    void setup();
    void encode(string filePath);

    

    
    OMX_HANDLETYPE decoder;
    OMX_BUFFERHEADERTYPE* decoderInputBuffer;

    
    static OMX_ERRORTYPE 
    decoderEventHandlerCallback(OMX_HANDLETYPE, 
                                OMX_PTR, 
                                OMX_EVENTTYPE, 
                                OMX_U32, OMX_U32, 
                                OMX_PTR);
    static OMX_ERRORTYPE 
    decoderEmptyBufferDone(OMX_IN OMX_HANDLETYPE, 
                           OMX_IN OMX_PTR, 
                           OMX_IN OMX_BUFFERHEADERTYPE*);
    
    static OMX_ERRORTYPE
    decoderFillBufferDone(OMX_IN OMX_HANDLETYPE,
                          OMX_IN OMX_PTR,
                          OMX_IN OMX_BUFFERHEADERTYPE*);
   
    
    void onDecoderPortSettingsChanged();
    void onDecoderEmptyBuffer();
    void onDecoderFillBuffer();
    
    
    OMX_HANDLETYPE resizer;
    OMX_BUFFERHEADERTYPE* resizerInputBuffer;
    OMX_BUFFERHEADERTYPE* resizerOutputBuffer;
    
    static OMX_ERRORTYPE 
    resizerEventHandlerCallback(OMX_HANDLETYPE, 
                                OMX_PTR, 
                                OMX_EVENTTYPE, 
                                OMX_U32, OMX_U32, 
                                OMX_PTR);
    static OMX_ERRORTYPE 
    resizerEmptyBufferDone(OMX_IN OMX_HANDLETYPE, 
                           OMX_IN OMX_PTR, 
                           OMX_IN OMX_BUFFERHEADERTYPE*);
    
    static OMX_ERRORTYPE
    resizerFillBufferDone(OMX_IN OMX_HANDLETYPE,
                          OMX_IN OMX_PTR,
                          OMX_IN OMX_BUFFERHEADERTYPE*);
    
    
    void onResizerPortSettingsChanged();
    void onResizerEmptyBuffer();
    void onResizerFillBuffer();
};
