#include "ofxOMXRecorder.h"




ofxOMXRecorder::ofxOMXRecorder()
{
    encoder = NULL;
    inputBuffer = NULL;
    outputBuffer = NULL;
    width = 0;
    height = 0;
    colorFormat = 0;
    pixelSize = 0;
    numMBps = 2;
    frameCounter = 0;
    stopRequested = false;
    startedRecording = false;
    createFileName();
    videoFileWritten = false;
    finishedRecording = false;
}

ofxOMXRecorder::~ofxOMXRecorder()
{
    if (encoder) 
    {
        OMX_ERRORTYPE error = OMX_ErrorNone;
        
        error = OMX_SendCommand(encoder, OMX_CommandStateSet, OMX_StateIdle, NULL);
        OMX_TRACE(error);
        
        error = OMX_SendCommand(encoder, OMX_CommandStateSet, OMX_StateLoaded, NULL);
        OMX_TRACE(error);
        
        error = DisableAllPortsForComponent(&encoder);
        OMX_TRACE(error);
        
        error = OMX_FreeBuffer(encoder, VIDEO_ENCODE_INPUT_PORT, inputBuffer);
        OMX_TRACE(error);
        inputBuffer = NULL;
        
        error = OMX_FreeBuffer(encoder, VIDEO_ENCODE_OUTPUT_PORT, outputBuffer);
        OMX_TRACE(error);
        outputBuffer = NULL;
        
        error = OMX_FreeHandle(encoder);
        OMX_TRACE(error);
    }
}
void ofxOMXRecorder::setup(int width_, int height_, int colorFormat_)
{
    
    width = width_;
    height = height_;
    colorFormat = colorFormat_;

    
    OMX_ERRORTYPE error = OMX_ErrorNone;
    error = OMX_Init();
    OMX_TRACE(error);
    
    
    OMX_CALLBACKTYPE encoderCallbacks;
    encoderCallbacks.EventHandler		= &ofxOMXRecorder::encoderEventHandlerCallback;
    encoderCallbacks.EmptyBufferDone	= &ofxOMXRecorder::encoderEmptyBufferDone;
    encoderCallbacks.FillBufferDone		= &ofxOMXRecorder::encoderFillBufferDone;
    
    error =OMX_GetHandle(&encoder, OMX_VIDEO_ENCODER, this , &encoderCallbacks);
    OMX_TRACE(error);
    
    error = DisableAllPortsForComponent(&encoder);
    OMX_TRACE(error);
    
    //Configure Input
    
    OMX_INIT_STRUCTURE(inputPortDefinition);
    inputPortDefinition.nPortIndex = VIDEO_ENCODE_INPUT_PORT;
    
    error =OMX_GetParameter(encoder, OMX_IndexParamPortDefinition, &inputPortDefinition);
    OMX_TRACE(error);
    
    inputPortDefinition.format.video.nFrameWidth = width;
    inputPortDefinition.format.video.nFrameHeight = height;
    inputPortDefinition.format.video.xFramerate = 30 << 16;
    inputPortDefinition.format.video.nSliceHeight = inputPortDefinition.format.video.nFrameHeight;
    inputPortDefinition.format.video.nStride = inputPortDefinition.format.video.nFrameWidth;
    //inputPortDefinition.format.video.eColorFormat = OMX_COLOR_FormatYUV420PackedPlanar;
    if (colorFormat == GL_RGB) 
    {
        pixelSize = width*height*3;
        inputPortDefinition.format.video.eColorFormat = OMX_COLOR_Format24bitBGR888;
    }
    if (colorFormat == GL_RGBA) 
    {
        pixelSize = width*height*4;
        inputPortDefinition.format.video.eColorFormat = OMX_COLOR_Format32bitABGR8888;
    }
    
    
    
    error =OMX_SetParameter(encoder, OMX_IndexParamPortDefinition, &inputPortDefinition);
    OMX_TRACE(error);

    
        
    // Configure encoder output buffer
   
    OMX_INIT_STRUCTURE(outputPortDefinition);
    outputPortDefinition.nPortIndex = VIDEO_ENCODE_OUTPUT_PORT;
    error =OMX_GetParameter(encoder, OMX_IndexParamPortDefinition, &outputPortDefinition);
    OMX_TRACE(error);
    
    int recordingBitRate = MEGABYTE_IN_BITS * numMBps;
    outputPortDefinition.format.video.nBitrate = recordingBitRate;

    error =OMX_SetParameter(encoder, OMX_IndexParamPortDefinition, &outputPortDefinition);
    OMX_TRACE(error);

    // Configure encoding bitrate
    OMX_VIDEO_PARAM_BITRATETYPE encodingBitrate;
    OMX_INIT_STRUCTURE(encodingBitrate);
    encodingBitrate.eControlRate = OMX_Video_ControlRateVariable;
    //encodingBitrate.eControlRate = OMX_Video_ControlRateConstant;
    
    encodingBitrate.nTargetBitrate = recordingBitRate;

    encodingBitrate.nPortIndex = VIDEO_ENCODE_OUTPUT_PORT;
    
    error = OMX_SetParameter(encoder, OMX_IndexParamVideoBitrate, &encodingBitrate);
    OMX_TRACE(error);
    
    // Configure encoding format
    OMX_VIDEO_PARAM_PORTFORMATTYPE encodingFormat;
    OMX_INIT_STRUCTURE(encodingFormat);
    encodingFormat.nPortIndex = VIDEO_ENCODE_OUTPUT_PORT;
    error = OMX_GetParameter(encoder, OMX_IndexParamVideoPortFormat, &encodingFormat);
    OMX_TRACE(error);
    
    encodingFormat.eCompressionFormat = OMX_VIDEO_CodingAVC;
    
    
    error = OMX_SetParameter(encoder, OMX_IndexParamVideoPortFormat, &encodingFormat);
    OMX_TRACE(error);


    
    error = OMX_SetParameter(encoder, OMX_IndexParamVideoPortFormat, &encodingFormat);
    OMX_TRACE(error);

    //Set encoder to Idle
    error = OMX_SendCommand(encoder, OMX_CommandStateSet, OMX_StateIdle, NULL);
    OMX_TRACE(error);
    
    
    OMX_PARAM_PORTDEFINITIONTYPE inPortDef = enablePortBuffers(encoder, &inputBuffer, VIDEO_ENCODE_INPUT_PORT);
    //ofLogVerbose(__func__) << "inputBuffer: " << GetBufferHeaderString(inputBuffer);
    //ofLogVerbose(__func__) << GetPortDefinitionString(inPortDef);
   
    
    OMX_PARAM_PORTDEFINITIONTYPE outPortDef = enablePortBuffers(encoder, &outputBuffer, VIDEO_ENCODE_OUTPUT_PORT);
    //ofLogVerbose(__func__) << "outputBuffer: " << GetBufferHeaderString(outputBuffer);
    //ofLogVerbose(__func__) << GetPortDefinitionString(outPortDef);
    
  
   
        
}

bool ofxOMXRecorder::isRecording()
{
    //ofLogVerbose() << "startedRecording: " << startedRecording;
    //ofLogVerbose() << "finishedRecording: " << finishedRecording;

    bool result = false;
    if (startedRecording)
    {
        if (finishedRecording) 
        {
            result = false;
        }else
        {
            result = true;
        }
    }
    return result;
}

void ofxOMXRecorder::createFileName()
{
    stringstream fileName;
    fileName << ofGetTimestampString() << "_";
    fileName << ".h264";
    filePath = ofToDataPath(fileName.str(), true);
}

void ofxOMXRecorder::startRecording()
{
    startRecording(filePath);
}

void ofxOMXRecorder::startRecording(string filePath_)
{
    if (filePath_.empty()) 
    {
        filePath = filePath_;
    }else
    {
        createFileName();
    }
    videoFileWritten = false;
    startedRecording = true;
    stopRequested = false;
    finishedRecording = false;
    OMX_ERRORTYPE error = OMX_SendCommand(encoder, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    OMX_TRACE(error);
    
}

void ofxOMXRecorder::stopRecording()
{
    stopRequested = true;
}


void ofxOMXRecorder::update(unsigned char* pixels)
{
    if (finishedRecording) 
    {
        return;
    }
    OMX_ERRORTYPE error;
    inputBuffer->pBuffer = pixels;
    inputBuffer->nFilledLen = pixelSize;
    //ofLogVerbose(__func__) << "inputBuffer: " << GetBufferHeaderString(inputBuffer);

    error = OMX_EmptyThisBuffer(encoder, inputBuffer);
    OMX_TRACE(error);
}

void ofxOMXRecorder::onPortSettingsChanged()
{
    ofLogVerbose(__func__) << "";
    
}
void ofxOMXRecorder::onEmptyBuffer()
{
    //ofLogVerbose(__func__) << "";
    OMX_ERRORTYPE error;
    error = OMX_FillThisBuffer(encoder, outputBuffer);
    OMX_TRACE(error);
    
}

void ofxOMXRecorder::onFillBuffer()
{
    ofLogVerbose(__func__) << "frameCounter: " << frameCounter;
    frameCounter++;
    recordingFileBuffer.append((const char*) outputBuffer->pBuffer + outputBuffer->nOffset, outputBuffer->nFilledLen);
    bool isKeyFrame =  (outputBuffer->nFlags & OMX_BUFFERFLAG_SYNCFRAME);
    if (isKeyFrame)
    {
        ofLogVerbose(__func__) << frameCounter << " IS KEYFRAME";
        
    }
    if (stopRequested && isKeyFrame) 
    {
        OMX_ERRORTYPE error = OMX_SendCommand(encoder, OMX_CommandStateSet, OMX_StateIdle, NULL);
        OMX_TRACE(error);
        finishedRecording = true;
        writeFile();
        frameCounter = 0;
        
    }
}

bool ofxOMXRecorder::didWriteFile()
{
    return videoFileWritten;
}

void ofxOMXRecorder::writeFile()
{
    
    videoFileWritten = ofBufferToFile(filePath, recordingFileBuffer, true);
    if(videoFileWritten)
    {
        ofFile file(filePath);
        files.push_back(file);
        ofLogError(__func__) << filePath << " SUCCESSFULLY WRITTEN";
    }else
    {
        ofLogError(__func__) << filePath << " COULD NOT BE WRITTEN";
    }
    recordingFileBuffer.clear();
    startedRecording = false;
    stopRequested = false;
}



OMX_PARAM_PORTDEFINITIONTYPE 
ofxOMXRecorder::enablePortBuffers(OMX_HANDLETYPE handle, OMX_BUFFERHEADERTYPE** targetBuffer, int portIndex)
{
    OMX_ERRORTYPE error;
    
    OMX_BUFFERHEADERTYPE* list = NULL;
    OMX_BUFFERHEADERTYPE** end = &list;
    
    OMX_PARAM_PORTDEFINITIONTYPE portdef;
    OMX_INIT_STRUCTURE(portdef);
    portdef.nPortIndex = portIndex;
    
    // work out buffer requirements, check port is in the right state
    error = OMX_GetParameter(handle, OMX_IndexParamPortDefinition, &portdef);
    OMX_TRACE(error);
    //ofLogVerbose(__func__) << GetPortDefinitionString(portdef);
    
    
    if( portdef.bEnabled != OMX_FALSE)
    {
        ofLogError(__func__) << "buffer requirements bEnabled" << portdef.bEnabled;
    }
    if( portdef.nBufferCountActual == 0)
    {
        ofLogError(__func__) << "buffer requirements nBufferCountActual" << portdef.nBufferCountActual;
    }
    
    if( portdef.nBufferSize == 0)
    {
        ofLogError(__func__) << "buffer requirements nBufferSize" << portdef.nBufferSize;
    }
    
    // check component is in the right state to accept buffers
    OMX_STATETYPE state;
    error = OMX_GetState(handle, &state);
    OMX_TRACE(error);
    if (!(state == OMX_StateIdle || state == OMX_StateExecuting || state == OMX_StatePause)) 
    {
        ofLogError(__func__) << "Incorrect state: " << GetOMXStateString(state);
    }
    
    // enable the port
    error = OMX_SendCommand(handle, OMX_CommandPortEnable, portIndex, NULL);
    OMX_TRACE(error);
    
    for (size_t i=0; i != portdef.nBufferCountActual; i++)
    {
        unsigned char *buf;
        buf = (unsigned char *)vcos_malloc_aligned(portdef.nBufferSize, portdef.nBufferAlignment, "whatever");
        
        
        if(!buf)
        {
            ofLogError(__func__) << "no buf";
            break;
        }
        
        error = OMX_UseBuffer(handle, end, portIndex, NULL, portdef.nBufferSize, buf);
        OMX_TRACE(error);
        if(error != OMX_ErrorNone)
        {
            vcos_free(buf);
        }
        end = (OMX_BUFFERHEADERTYPE **) &((*end)->pAppPrivate);
    }
    
    *targetBuffer = list;
    
    return portdef;
    
}


OMX_ERRORTYPE 
ofxOMXRecorder::encoderEventHandlerCallback(OMX_HANDLETYPE hComponent,
                                         OMX_PTR pAppData,
                                         OMX_EVENTTYPE event,
                                         OMX_U32 nData1,
                                         OMX_U32 nData2,
                                         OMX_PTR pEventData)
{
    ofxOMXRecorder *recorder = static_cast<ofxOMXRecorder*>(pAppData);
    
    //ofLogVerbose(__func__) << GetEventString(event);

    if(event == OMX_EventError)
    {
        //ofLogVerbose(__func__) << GetOMXErrorString((OMX_ERRORTYPE) nData1);
        
    }
    if (event == OMX_EventPortSettingsChanged) 
    {
        recorder->onPortSettingsChanged();
    }
    return OMX_ErrorNone;
};


OMX_ERRORTYPE ofxOMXRecorder::encoderEmptyBufferDone(OMX_IN OMX_HANDLETYPE hComponent, 
                                                  OMX_IN OMX_PTR pAppData, 
                                                  OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{
    ofxOMXRecorder *recorder = static_cast<ofxOMXRecorder*>(pAppData);
    recorder->onEmptyBuffer();
    //ofLogVerbose(__func__) << "";
    return OMX_ErrorNone;
}


OMX_ERRORTYPE ofxOMXRecorder::encoderFillBufferDone(OMX_IN OMX_HANDLETYPE hComponent, 
                                                 OMX_IN OMX_PTR pAppData, 
                                                 OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{	
    ofxOMXRecorder *recorder = static_cast<ofxOMXRecorder*>(pAppData);
    //ofLogVerbose(__func__) << "";
    recorder->onFillBuffer();
    return OMX_ErrorNone;
}