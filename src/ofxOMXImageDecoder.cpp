#include "ofxOMXImageDecoder.h"


ofxOMXImageDecoder::ofxOMXImageDecoder()
{
    decoder = NULL;
    resizer = NULL;
    decoderInputBuffer = NULL;
    resizerInputBuffer = NULL;
    resizerOutputBuffer = NULL;
}

ofxOMXImageDecoder::~ofxOMXImageDecoder()
{
     if (decoder) 
     {
         OMX_ERRORTYPE error = OMX_ErrorNone;
         
         error = OMX_SendCommand(decoder, OMX_CommandStateSet, OMX_StateIdle, NULL);
         OMX_TRACE(error);
         
         error = OMX_SendCommand(decoder, OMX_CommandStateSet, OMX_StateLoaded, NULL);
         OMX_TRACE(error);
         
         error = DisableAllPortsForComponent(&decoder);
         OMX_TRACE(error);
         
         error = OMX_FreeBuffer(decoder, IMAGE_DECODER_INPUT_PORT, decoderInputBuffer);
         OMX_TRACE(error);
         decoderInputBuffer = NULL;
         
         error = OMX_FreeHandle(decoder);
         OMX_TRACE(error);
     }
    if (resizer) 
    {
        OMX_ERRORTYPE error = OMX_ErrorNone;
        
        error = OMX_SendCommand(resizer, OMX_CommandStateSet, OMX_StateIdle, NULL);
        OMX_TRACE(error);
        
        error = OMX_SendCommand(resizer, OMX_CommandStateSet, OMX_StateLoaded, NULL);
        OMX_TRACE(error);
        
        error = DisableAllPortsForComponent(&resizer);
        OMX_TRACE(error);
        
        error = OMX_FreeBuffer(resizer, RESIZER_INPUT_PORT, resizerInputBuffer);
        OMX_TRACE(error);
        resizerInputBuffer = NULL;
        
        error = OMX_FreeBuffer(resizer, RESIZER_OUTPUT_PORT, resizerOutputBuffer);
        OMX_TRACE(error);
        resizerOutputBuffer = NULL;
        
        error = OMX_FreeHandle(resizer);
        OMX_TRACE(error);
    }

}

void ofxOMXImageDecoder::setup()
{
    OMX_ERRORTYPE error = OMX_ErrorNone;
    error = OMX_Init();
    OMX_TRACE(error);
    
    
    OMX_CALLBACKTYPE decoderCallbacks;
    decoderCallbacks.EventHandler		= &ofxOMXImageDecoder::decoderEventHandlerCallback;
    decoderCallbacks.EmptyBufferDone	= &ofxOMXImageDecoder::decoderEmptyBufferDone;
    decoderCallbacks.FillBufferDone		= &ofxOMXImageDecoder::decoderFillBufferDone;
    
    error =OMX_GetHandle(&decoder, OMX_IMAGE_DECODER, this , &decoderCallbacks);
    OMX_TRACE(error);
    
    error = DisableAllPortsForComponent(&decoder);
    OMX_TRACE(error);
    
    OMX_PARAM_PORTDEFINITIONTYPE decoderInputPortDefinition;
    //OMX_PARAM_PORTDEFINITIONTYPE decoderOutputPortDefinition;
    OMX_INIT_STRUCTURE(decoderInputPortDefinition);
    decoderInputPortDefinition.nPortIndex = IMAGE_DECODER_INPUT_PORT;
    
    error =OMX_GetParameter(decoder, OMX_IndexParamPortDefinition, &decoderInputPortDefinition);
    OMX_TRACE(error);
    


    
    OMX_CALLBACKTYPE resizerCallbacks;
    resizerCallbacks.EventHandler		= &ofxOMXImageDecoder::resizerEventHandlerCallback;
    resizerCallbacks.EmptyBufferDone	= &ofxOMXImageDecoder::resizerEmptyBufferDone;
    resizerCallbacks.FillBufferDone		= &ofxOMXImageDecoder::resizerFillBufferDone;
    
    error =OMX_GetHandle(&resizer, OMX_RESIZER, this , &resizerCallbacks);
    OMX_TRACE(error);
    
    error = DisableAllPortsForComponent(&resizer);
    OMX_TRACE(error);
    
    OMX_PARAM_PORTDEFINITIONTYPE resizerInputPortDefinition;
    OMX_INIT_STRUCTURE(resizerInputPortDefinition);
    resizerInputPortDefinition.nPortIndex = RESIZER_INPUT_PORT;
    
    error =OMX_GetParameter(resizer, OMX_IndexParamPortDefinition, &resizerInputPortDefinition);
    OMX_TRACE(error);
    
    
    //Allocate buffers
    //Set decoder to Idle
    error = OMX_SendCommand(decoder, OMX_CommandStateSet, OMX_StateIdle, NULL);
    OMX_TRACE(error);
    
    OMX_PARAM_PORTDEFINITIONTYPE decoderInputPortDef = EnablePortBuffers(decoder, &decoderInputBuffer, IMAGE_DECODER_INPUT_PORT);
    ofLogVerbose(__func__) << "decoderInputBuffer: " << GetBufferHeaderString(decoderInputBuffer);
    ofLogVerbose(__func__) << "decoderInputPortDef: " << GetPortDefinitionString(decoderInputPortDef);
    
    error = OMX_SendCommand(resizer, OMX_CommandStateSet, OMX_StateIdle, NULL);
    OMX_TRACE(error);
    
    OMX_PARAM_PORTDEFINITIONTYPE resizerInputPortDef = EnablePortBuffers(resizer, &resizerInputBuffer, RESIZER_INPUT_PORT);
    ofLogVerbose(__func__) << "resizerInputBuffer: " << GetBufferHeaderString(resizerInputBuffer);
    ofLogVerbose(__func__) << "resizerInputPortDef: " << GetPortDefinitionString(resizerInputPortDef);
    
    
    OMX_PARAM_PORTDEFINITIONTYPE resizerOutPortDef = EnablePortBuffers(resizer, &resizerOutputBuffer, RESIZER_OUTPUT_PORT);
    ofLogVerbose(__func__) << "resizerOutputBuffer: " << GetBufferHeaderString(resizerOutputBuffer);
    ofLogVerbose(__func__) << "resizerOutPortDef: " << GetPortDefinitionString(resizerOutPortDef);
    
}


void ofxOMXImageDecoder::encode(string filePath)
{
    ofLogVerbose(__func__) << "";
    
}


void ofxOMXImageDecoder::onResizerPortSettingsChanged()
{
    ofLogVerbose(__func__) << "";
    
}

void ofxOMXImageDecoder::onResizerEmptyBuffer()
{
    //ofLogVerbose(__func__) << "";
    OMX_ERRORTYPE error;
    //error = OMX_FillThisBuffer(decoder, outputBuffer);
    OMX_TRACE(error);
    
}

void ofxOMXImageDecoder::onResizerFillBuffer()
{
    ofLogVerbose(__func__) << "";
    
}


void ofxOMXImageDecoder::onDecoderPortSettingsChanged()
{
    ofLogVerbose(__func__) << "";
    
}

void ofxOMXImageDecoder::onDecoderEmptyBuffer()
{
    //ofLogVerbose(__func__) << "";
    OMX_ERRORTYPE error;
    //error = OMX_FillThisBuffer(decoder, outputBuffer);
    OMX_TRACE(error);
    
}

void ofxOMXImageDecoder::onDecoderFillBuffer()
{
    ofLogVerbose(__func__) << "";

}


OMX_ERRORTYPE 
ofxOMXImageDecoder::decoderEventHandlerCallback(OMX_HANDLETYPE hComponent,
                                            OMX_PTR pAppData,
                                            OMX_EVENTTYPE event,
                                            OMX_U32 nData1,
                                            OMX_U32 nData2,
                                            OMX_PTR pEventData)
{
    ofxOMXImageDecoder *imageDecoder = static_cast<ofxOMXImageDecoder*>(pAppData);
    
    //ofLogVerbose(__func__) << GetEventString(event);
    
    if(event == OMX_EventError)
    {
        //ofLogVerbose(__func__) << GetOMXErrorString((OMX_ERRORTYPE) nData1);
        
    }
    if (event == OMX_EventPortSettingsChanged) 
    {
        imageDecoder->onDecoderPortSettingsChanged();
    }
    return OMX_ErrorNone;
};


OMX_ERRORTYPE 
ofxOMXImageDecoder::decoderEmptyBufferDone(OMX_IN OMX_HANDLETYPE hComponent, 
                                                     OMX_IN OMX_PTR pAppData, 
                                                     OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{
    ofxOMXImageDecoder *imageDecoder = static_cast<ofxOMXImageDecoder*>(pAppData);
    imageDecoder->onDecoderEmptyBuffer();
    //ofLogVerbose(__func__) << "";
    return OMX_ErrorNone;
}


OMX_ERRORTYPE 
ofxOMXImageDecoder::decoderFillBufferDone(OMX_IN OMX_HANDLETYPE hComponent, 
                                                    OMX_IN OMX_PTR pAppData, 
                                                    OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{	
    ofxOMXImageDecoder *imageDecoder = static_cast<ofxOMXImageDecoder*>(pAppData);

    //ofLogVerbose(__func__) << "";
    imageDecoder->onDecoderFillBuffer();
    return OMX_ErrorNone;
}

OMX_ERRORTYPE 
ofxOMXImageDecoder::resizerEventHandlerCallback(OMX_HANDLETYPE hComponent,
                                                OMX_PTR pAppData,
                                                OMX_EVENTTYPE event,
                                                OMX_U32 nData1,
                                                OMX_U32 nData2,
                                                OMX_PTR pEventData)
{
    ofxOMXImageDecoder *imageDecoder = static_cast<ofxOMXImageDecoder*>(pAppData);
    
    //ofLogVerbose(__func__) << GetEventString(event);
    
    if(event == OMX_EventError)
    {
        //ofLogVerbose(__func__) << GetOMXErrorString((OMX_ERRORTYPE) nData1);
        
    }
    if (event == OMX_EventPortSettingsChanged) 
    {
        imageDecoder->onResizerPortSettingsChanged();
    }
    return OMX_ErrorNone;
};


OMX_ERRORTYPE 
ofxOMXImageDecoder::resizerEmptyBufferDone(OMX_IN OMX_HANDLETYPE hComponent, 
                                           OMX_IN OMX_PTR pAppData, 
                                           OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{
    ofxOMXImageDecoder *imageDecoder = static_cast<ofxOMXImageDecoder*>(pAppData);
    imageDecoder->onResizerEmptyBuffer();
    //ofLogVerbose(__func__) << "";
    return OMX_ErrorNone;
}


OMX_ERRORTYPE 
ofxOMXImageDecoder::resizerFillBufferDone(OMX_IN OMX_HANDLETYPE hComponent, 
                                          OMX_IN OMX_PTR pAppData, 
                                          OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{	
    ofxOMXImageDecoder *imageDecoder = static_cast<ofxOMXImageDecoder*>(pAppData);
    
    //ofLogVerbose(__func__) << "";
    imageDecoder->onResizerFillBuffer();
    return OMX_ErrorNone;
}
