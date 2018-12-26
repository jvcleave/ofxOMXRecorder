#include "ofxOMXRecorder.h"




ofxOMXRecorder::ofxOMXRecorder()
{
    resetValues();
}

void ofxOMXRecorder::resetValues()
{
    encoder = NULL;
    encoderInputBuffer = NULL;
    resizeInputBuffer = NULL;
    encoderOutputBuffer = NULL;
    listener = NULL;
    pixelSize = 0;
    frameCounter = 0;
    stopRequested = false;
    startedRecording = false;
    absoluteFilePath = "";
    finishedRecording = false;
    isOpen = false;
    canTakeFrame = false;

}

void ofxOMXRecorder::close()
{
    //TODO: file may not be written
    if (isRecording()) 
    {
        stopRecording();
    }else
    {
        destroyEncoder();
    }
}

void ofxOMXRecorder::destroyEncoder()
{
    if (encoder) 
    {
        OMX_ERRORTYPE error = OMX_ErrorNone;
        
        error = DisableAllPortsForComponent(&encoder);
        OMX_TRACE(error);
        
        error = OMX_SendCommand(encoder, OMX_CommandFlush, OMX_ALL, NULL);
        OMX_TRACE(error);


        if(encoderInputBuffer)
        {
            error = OMX_FreeBuffer(encoder, VIDEO_ENCODE_INPUT_PORT, encoderInputBuffer);
            OMX_TRACE(error);
            encoderInputBuffer = NULL; 
        }
        if(encoderOutputBuffer)
        {
            error = OMX_FreeBuffer(encoder, VIDEO_ENCODE_OUTPUT_PORT, encoderOutputBuffer);
            OMX_TRACE(error);
            encoderOutputBuffer = NULL; 
        }
        
        error = OMX_FreeHandle(encoder);
        OMX_TRACE(error);
        
        
    }
    resetValues();
}

ofxOMXRecorder::~ofxOMXRecorder()
{
    destroyEncoder();    
}




void ofxOMXRecorder::setup(ofxOMXRecorderSettings settings_)
{
    settings = settings_;
    settings.outputWidth = VCOS_ALIGN_UP(settings.outputWidth, 64);
    settings.outputHeight = VCOS_ALIGN_UP(settings.outputHeight, 64);

    OMX_ERRORTYPE error = OMX_ErrorNone;
    error = OMX_Init();
    OMX_TRACE(error);
    
    OMX_CALLBACKTYPE resizerCallbacks;
    resizerCallbacks.EventHandler    = &ofxOMXRecorder::resizerEventHandlerCallback;
    resizerCallbacks.EmptyBufferDone = &ofxOMXRecorder::resizerEmptyBufferDone;
    resizerCallbacks.FillBufferDone  = &ofxOMXRecorder::resizerFillBufferDone;
    
    error =OMX_GetHandle(&resizer, OMX_RESIZER, this, &resizerCallbacks);
    OMX_TRACE(error);
    
    
    
    error = DisableAllPortsForComponent(&resizer);
    OMX_TRACE(error);
    
    
    OMX_PARAM_PORTDEFINITIONTYPE resizerInputPort;
    OMX_INIT_STRUCTURE(resizerInputPort);
    resizerInputPort.nPortIndex = RESIZER_INPUT_PORT;
    
    error =OMX_GetParameter(resizer,
                            OMX_IndexParamPortDefinition,
                            &resizerInputPort);
    OMX_TRACE(error);
    
    resizerInputPort.format.image.nFrameWidth   =   settings.width;
    resizerInputPort.format.image.nFrameHeight  =   settings.height;
    resizerInputPort.format.image.nSliceHeight  =   settings.height;
    resizerInputPort.format.image.eColorFormat  =   OMX_COLOR_Format32bitABGR8888;
    //Stride is byte-per-pixel*width
    resizerInputPort.format.image.nStride       =   settings.width * 4;
    
    error =OMX_SetParameter(resizer, OMX_IndexParamPortDefinition, &resizerInputPort);
    OMX_TRACE(error);
    
    error =OMX_GetParameter(resizer, OMX_IndexParamPortDefinition, &resizerInputPort);
    OMX_TRACE(error);
    ofLogVerbose(__func__) << "resizerInputPort: " << GetPortDefinitionString(resizerInputPort); 
    
    OMX_PARAM_PORTDEFINITIONTYPE resizerOutputPort;
    OMX_INIT_STRUCTURE(resizerOutputPort);
    resizerOutputPort.nPortIndex = RESIZER_OUTPUT_PORT;
    
    
    error =OMX_GetParameter(resizer,
                            OMX_IndexParamPortDefinition,
                            &resizerOutputPort);
    OMX_TRACE(error);
    resizerOutputPort.format.image.nFrameWidth  =   settings.outputWidth;
    resizerOutputPort.format.image.nFrameHeight =   settings.outputHeight;
    resizerOutputPort.format.image.nSliceHeight =   settings.outputHeight;
    resizerOutputPort.format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;
    resizerOutputPort.format.image.eColorFormat = OMX_COLOR_Format32bitABGR8888;
    resizerOutputPort.format.image.nStride      =   settings.outputWidth*4;
    
    error =OMX_SetParameter(resizer, OMX_IndexParamPortDefinition, &resizerOutputPort);
    OMX_TRACE(error);
    
    
    OMX_CALLBACKTYPE encoderCallbacks;
    encoderCallbacks.EventHandler    = &ofxOMXRecorder::encoderEventHandlerCallback;
    encoderCallbacks.EmptyBufferDone = &ofxOMXRecorder::encoderEmptyBufferDone;
    encoderCallbacks.FillBufferDone  = &ofxOMXRecorder::encoderFillBufferDone;
    
    error =OMX_GetHandle(&encoder, OMX_VIDEO_ENCODER, this , &encoderCallbacks);
    OMX_TRACE(error);
    
    error = DisableAllPortsForComponent(&encoder);
    OMX_TRACE(error);
    
    //Configure Input
    
    OMX_PARAM_PORTDEFINITIONTYPE encoderInputPort;
    OMX_INIT_STRUCTURE(encoderInputPort);
    encoderInputPort.nPortIndex = VIDEO_ENCODE_INPUT_PORT;
    
    error =OMX_GetParameter(encoder, OMX_IndexParamPortDefinition, &encoderInputPort);
    OMX_TRACE(error);
    
    encoderInputPort.format.video.nFrameWidth    =   settings.outputWidth;
    encoderInputPort.format.video.nFrameHeight   =   settings.outputHeight;
    encoderInputPort.format.video.xFramerate     =   settings.fps << 16;
    encoderInputPort.format.video.nSliceHeight   =   settings.outputHeight;
    encoderInputPort.format.video.nStride        =   settings.outputWidth*4;
    //encoderInputPort.format.video.eColorFormat = OMX_COLOR_FormatYUV420PackedPlanar;
    if (settings.colorFormat == GL_RGB) 
    {
        pixelSize = settings.outputWidth * settings.outputHeight * 3;
        encoderInputPort.format.video.eColorFormat = OMX_COLOR_Format24bitBGR888;
    }
    if (settings.colorFormat == GL_RGBA) 
    {
        pixelSize = settings.outputWidth * settings.outputHeight * 4;
        encoderInputPort.format.video.eColorFormat = OMX_COLOR_Format32bitABGR8888;
    }
    
    
    
    error =OMX_SetParameter(encoder, OMX_IndexParamPortDefinition, &encoderInputPort);
    OMX_TRACE(error);
    
    
    OMX_PARAM_PORTDEFINITIONTYPE encoderOutputPort;
    // Configure encoder output buffer
    OMX_INIT_STRUCTURE(encoderOutputPort);
    encoderOutputPort.nPortIndex = VIDEO_ENCODE_OUTPUT_PORT;
    error =OMX_GetParameter(encoder, OMX_IndexParamPortDefinition, &encoderOutputPort);
    OMX_TRACE(error);
    
    int recordingBitRate = int(MEGABYTE_IN_BITS * settings.bitrateMegabytesPerSecond);
    encoderOutputPort.format.video.nBitrate = recordingBitRate;
    
    error =OMX_SetParameter(encoder, OMX_IndexParamPortDefinition, &encoderOutputPort);
    OMX_TRACE(error);
    
    /*
    OMX_PARAM_BRCMVIDEOAVCSEIENABLETYPE sei_param;
    OMX_INIT_STRUCTURE(sei_param);
    sei_param.nPortIndex = VIDEO_ENCODE_OUTPUT_PORT;
    sei_param.bEnable = OMX_TRUE;
    
    error =OMX_SetParameter(encoder, OMX_IndexParamBrcmVideoAVCSEIEnable, &sei_param);
    OMX_TRACE(error);
    
    OMX_CONFIG_PORTBOOLEANTYPE AVCInlineHeaderEnable;
    OMX_INIT_STRUCTURE(AVCInlineHeaderEnable);
    AVCInlineHeaderEnable.nPortIndex = VIDEO_ENCODE_OUTPUT_PORT;
    AVCInlineHeaderEnable.bEnabled = OMX_TRUE;
    error =OMX_SetParameter(encoder, OMX_IndexParamBrcmVideoAVCInlineHeaderEnable, &AVCInlineHeaderEnable);
    OMX_TRACE(error);
    */
    
    
    
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
    
    setKeyFrameInterval(settings.keyFrameInterval);
    
    
    //Set encoder to Idle
    error = OMX_SendCommand(resizer, OMX_CommandStateSet, OMX_StateIdle, NULL);
    OMX_TRACE(error);
    
    //Set encoder to Idle
    error = OMX_SendCommand(encoder, OMX_CommandStateSet, OMX_StateIdle, NULL);
    OMX_TRACE(error);
    
    
    OMX_PARAM_PORTDEFINITIONTYPE resizerInPortDef  = enablePortBuffers(resizer, &resizeInputBuffer, RESIZER_INPUT_PORT);
    OMX_PARAM_PORTDEFINITIONTYPE resizerOutPortDef = enablePortBuffers(resizer, &resizeOutputBuffer, RESIZER_OUTPUT_PORT);
    OMX_PARAM_PORTDEFINITIONTYPE encoderInPortDef  = enablePortBuffers(encoder, &encoderInputBuffer, VIDEO_ENCODE_INPUT_PORT);
    OMX_PARAM_PORTDEFINITIONTYPE encoderOutPortDef  = enablePortBuffers(encoder, &encoderOutputBuffer, VIDEO_ENCODE_OUTPUT_PORT);

    OMX_TRACE(error);
    ofLogVerbose(__func__) << PrintPortDefinition(resizer, RESIZER_INPUT_PORT);
    ofLogVerbose(__func__) << PrintPortDefinition(resizer, RESIZER_OUTPUT_PORT);
    ofLogVerbose(__func__) << PrintPortDefinition(encoder, VIDEO_ENCODE_INPUT_PORT);
    ofLogVerbose(__func__) << PrintPortDefinition(encoder, VIDEO_ENCODE_OUTPUT_PORT);
    
    
    error = SetComponentState(resizer, OMX_StateExecuting);
    OMX_TRACE(error);
    
    error = SetComponentState(encoder, OMX_StateExecuting);
    OMX_TRACE(error);

    startThread();
    isOpen = true;
}


void ofxOMXRecorder::threadedFunction()
{
    
}
void ofxOMXRecorder::startRecording(string absoluteFilePath_) //default ""
{

    if (!isOpen) 
    {
        setup(settings);
    }
    if (isRecording())
    {
        return;
    }
    absoluteFilePath = absoluteFilePath_;
    
    startedRecording = true;
    stopRequested = false;
    finishedRecording = false;
    canTakeFrame = true;

    //Start encoder
    if(listener)
    {
        listener->onRecordStart();
    }
}   
    
void ofxOMXRecorder::setKeyFrameInterval(int keyFrameInterval)
{
    settings.keyFrameInterval = keyFrameInterval;
    OMX_PARAM_U32TYPE keyFrameIntervalConfig;
    OMX_INIT_STRUCTURE(keyFrameIntervalConfig);
    keyFrameIntervalConfig.nPortIndex = VIDEO_ENCODE_OUTPUT_PORT;
    OMX_ERRORTYPE error = OMX_GetParameter(encoder, OMX_IndexConfigBrcmVideoIntraPeriod, &keyFrameIntervalConfig);
    OMX_TRACE(error);
    keyFrameIntervalConfig.nU32 = settings.keyFrameInterval;
    error = OMX_SetParameter(encoder, OMX_IndexConfigBrcmVideoIntraPeriod, &keyFrameIntervalConfig);
    OMX_TRACE(error);
}


bool ofxOMXRecorder::isRecording()
{
    //ofLogVerbose() << "startedRecording: " << startedRecording;
    //ofLogVerbose() << "finishedRecording: " << finishedRecording;

    bool result = false;
    if(!isOpen)
    {
        //ofLogNotice(__func__) << "NOT OPEN";
        return result;
    }
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



void ofxOMXRecorder::stopRecording()
{
    stopRequested = true;
}


void ofxOMXRecorder::update(unsigned char* pixels)
{
    if (!isOpen) 
    {
        ofLogError(__func__) << "NOT OPEN";
        return;
    }
    if (!pixels) 
    {
        ofLogError(__func__) << "pixels is NULL";
        return;
    }
    if (finishedRecording) 
    {
        return;
    }
    canTakeFrame = false;
    resizeInputBuffer->pBuffer = pixels;
    resizeInputBuffer->nFilledLen = pixelSize;
    //ofLogVerbose(__func__) << "resizeInputBuffer: " << GetBufferHeaderString(resizeInputBuffer);

    OMX_ERRORTYPE error = OMX_EmptyThisBuffer(resizer, resizeInputBuffer);
    OMX_TRACE(error);
    
    error = OMX_FillThisBuffer(resizer, resizeOutputBuffer);
    OMX_TRACE(error);
   
    

}

OMX_ERRORTYPE 
ofxOMXRecorder::resizerEventHandlerCallback(OMX_HANDLETYPE hComponent, 
                                            OMX_PTR pAppData, 
                                            OMX_EVENTTYPE event, 
                                            OMX_U32 nData1, OMX_U32 nData2, 
                                            OMX_PTR pEventData)
{
    //ofLog() << "resizerEventHandlerCallback: " << DebugEventHandlerString(hComponent, event, nData1, nData2, pEventData);
    
    return OMX_ErrorNone;
    
}


OMX_ERRORTYPE 
ofxOMXRecorder::resizerEmptyBufferDone(OMX_HANDLETYPE resizer, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE*)
{
    return OMX_ErrorNone;

}

OMX_ERRORTYPE 
ofxOMXRecorder::resizerFillBufferDone(OMX_HANDLETYPE, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE*)
{
    //ofLogNotice(__func__) << endl;
    ofxOMXRecorder* recorder = static_cast<ofxOMXRecorder*>(pAppData);
    recorder->encoderInputBuffer->pBuffer =    recorder->resizeOutputBuffer->pBuffer;
    recorder->encoderInputBuffer->nFilledLen = recorder->resizeOutputBuffer->nFilledLen;

    OMX_ERRORTYPE error = OMX_EmptyThisBuffer(recorder->encoder, recorder->encoderInputBuffer);
    OMX_TRACE(error);
    
    return OMX_ErrorNone;
    
}

OMX_ERRORTYPE 
ofxOMXRecorder::encoderEventHandlerCallback(OMX_HANDLETYPE hComponent, 
                                            OMX_PTR pAppData, 
                                            OMX_EVENTTYPE event, 
                                            OMX_U32 nData1, OMX_U32 nData2, 
                                            OMX_PTR pEventData)
{
    //ofLog() << "encoderEventHandlerCallback: " << DebugEventHandlerString(hComponent, event, nData1, nData2, pEventData);
    return OMX_ErrorNone;
    
}

OMX_ERRORTYPE ofxOMXRecorder::encoderEmptyBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
    ofxOMXRecorder* recorder = static_cast<ofxOMXRecorder*>(pAppData);
    OMX_ERRORTYPE error = OMX_FillThisBuffer(hComponent, recorder->encoderOutputBuffer);
    OMX_TRACE(error);
    return error;
}







OMX_ERRORTYPE ofxOMXRecorder::encoderFillBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* encoderOutputBuffer)
{    
    ofxOMXRecorder* recorder = static_cast<ofxOMXRecorder*>(pAppData);

    bool isKeyFrame =  (encoderOutputBuffer->nFlags & OMX_BUFFERFLAG_SYNCFRAME);
    if (isKeyFrame)
    {
        ofLogVerbose(__func__) << recorder->frameCounter << " IS KEYFRAME";

    }
    
    if (recorder->stopRequested && isKeyFrame) 
    {
       
        recorder->finishedRecording = true;
        recorder->writeFile();
    }else
    {
        recorder->frameCounter++;
        recorder->recordingFileBuffer.append((const char*) encoderOutputBuffer->pBuffer + encoderOutputBuffer->nOffset, encoderOutputBuffer->nFilledLen);
        ofLog() << "frameCounter: " << recorder->frameCounter << " : " << recorder->recordingFileBuffer.size();
        if(recorder->listener)
        {
            recorder->listener->onFrameRecorded();
        }
        recorder->canTakeFrame = true;

    }

    return OMX_ErrorNone;
}




string ofxOMXRecorder::createFileName()
{
    stringstream fileName;
    fileName << ofGetTimestampString();
    if (settings.enablePrettyFileName) 
    {
        fileName << "_"<< settings.width << "x" << settings.height << "_";
        fileName << settings.fps << "fps_";
        fileName << settings.bitrateMegabytesPerSecond << "MBps_";
        fileName << frameCounter << "frames";
    }
    fileName << ".h264";
    return fileName.str();
    
}



void ofxOMXRecorder::writeFile()
{
    
    OMX_ERRORTYPE error = OMX_SendCommand(encoder, OMX_CommandStateSet, OMX_StateIdle, NULL);
    OMX_TRACE(error);
    
    
    error = OMX_SendCommand(resizer, OMX_CommandStateSet, OMX_StateIdle, NULL);
    OMX_TRACE(error);
    
    
    if(absoluteFilePath.empty())
    {
        absoluteFilePath = ofToDataPath(createFileName(), true);
    }

    bool didWriteFile = ofBufferToFile(absoluteFilePath, recordingFileBuffer, true);
    if(didWriteFile)
    {
        if(listener)
        {
            listener->onFileWritten(absoluteFilePath);
        }
        
        ofFile file(absoluteFilePath);
        recordings.push_back(file);
        ofLogVerbose(__func__) << absoluteFilePath << " SUCCESSFULLY WRITTEN";
    }else
    {
        ofLogError(__func__) << absoluteFilePath << " COULD NOT BE WRITTEN";
    }
    recordingFileBuffer.clear();
    frameCounter = 0;
    absoluteFilePath = "";
    startedRecording = false;
    stopRequested = false;
    destroyEncoder();
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

    
    ofLogVerbose(__func__) << GetPortDefinitionString(portdef);

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
    ofLogNotice(__func__) << "ALLOCATING " << portdef.nBufferCountActual << " BUFFERS";
    
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





