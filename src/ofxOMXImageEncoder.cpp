#include "ofxOMXImageEncoder.h"


//#define DEVEL_MODE 1

#if defined(DEVEL_MODE)
    #define __chan__ __func__
    #define LOG_LINE ofLogVerbose(__chan__) << __LINE__ << endl;
    #define PORT_CHECK_NO_BUFFER(...) LOG_LINE checkPorts(false); 
    #define PORT_CHECK(...) LOG_LINE checkPorts(); 
#else
    #define PORT_CHECK(...)
    #define __chan__ "ofxOMXImageEncoder"
#endif







ofxOMXImageEncoder::ofxOMXImageEncoder()
{
    
    ofSetLogLevel("ofxOMXImageEncoder", OF_LOG_ERROR);
    resetValues();
}



#pragma mark SETUP
void ofxOMXImageEncoder::setup(ofxOMXImageEncoderSettings settings_)
{
    settings = settings_;
    settings.validate();
    int numColors = 0;
    if (settings.colorFormat == GL_RGB) 
    {
        numColors = 3;
        
    }
    if (settings.colorFormat == GL_RGBA) 
    {
        numColors = 4;
    }
    pixelSize = settings.width * settings.height * numColors;

    OMX_ERRORTYPE error = OMX_ErrorNone;
    error = OMX_Init();
    OMX_TRACE(error);
    
    
    codingType = (OMX_IMAGE_CODINGTYPE)settings.imageType;
    
    bool needResize = false;
    
    OMX_CALLBACKTYPE resizerCallbacks;
    resizerCallbacks.EventHandler		= &ofxOMXImageEncoder::resizerEventHandlerCallback;
    resizerCallbacks.EmptyBufferDone	= &ofxOMXImageEncoder::resizerEmptyBufferDone;
    resizerCallbacks.FillBufferDone		= &ofxOMXImageEncoder::resizerFillBufferDone;
    
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
    
    
    
    if (settings.colorFormat == GL_RGB) 
    {
        resizerInputPort.format.image.eColorFormat = OMX_COLOR_Format24bitBGR888;
        
    }
    if (settings.colorFormat == GL_RGBA) 
    {
        resizerInputPort.format.image.eColorFormat = OMX_COLOR_Format32bitABGR8888;
    }
    
    
    resizerInputPort.format.image.nFrameWidth   =   settings.width;
    resizerInputPort.format.image.nFrameHeight  =   settings.height;
    resizerInputPort.format.image.nSliceHeight  =   settings.height;
    //Stride is byte-per-pixel*width
    resizerInputPort.format.image.nStride       =   settings.width * numColors;

    
    error =OMX_SetParameter(resizer, OMX_IndexParamPortDefinition, &resizerInputPort);
    OMX_TRACE(error);
    
    ofLogVerbose(__chan__) << "resizerInputPort: " << GetPortDefinitionString(resizerInputPort); 
    
    
    
    OMX_PARAM_PORTDEFINITIONTYPE resizerOutputPort;
    OMX_INIT_STRUCTURE(resizerOutputPort);
    resizerOutputPort.nPortIndex = RESIZER_OUTPUT_PORT;
    
    
    error =OMX_GetParameter(resizer,
                            OMX_IndexParamPortDefinition,
                            &resizerOutputPort);
    OMX_TRACE(error);
    
    resizerOutputPort.format.image.nFrameWidth  =   settings.outputWidth;
    resizerOutputPort.format.image.nFrameHeight =   settings.outputHeight;
    resizerOutputPort.format.image.nSliceHeight =   (settings.outputHeight+15) & ~15;
    resizerOutputPort.format.image.nStride      =   settings.outputWidth*numColors;
 
    
    error =OMX_SetParameter(resizer,
                            OMX_IndexParamPortDefinition,
                            &resizerOutputPort);
    OMX_TRACE(error);


    OMX_CALLBACKTYPE encoderCallbacks;
    encoderCallbacks.EventHandler    = &ofxOMXImageEncoder::encoderEventHandlerCallback;
    encoderCallbacks.EmptyBufferDone = &ofxOMXImageEncoder::encoderEmptyBufferDone;
    encoderCallbacks.FillBufferDone  = &ofxOMXImageEncoder::encoderFillBufferDone;
    
    error =OMX_GetHandle(&encoder, OMX_IMAGE_ENCODER, this , &encoderCallbacks);
    OMX_TRACE(error);
    
    error = DisableAllPortsForComponent(&encoder);
    OMX_TRACE(error);
    
    OMX_PARAM_PORTDEFINITIONTYPE encoderInputPort;
    OMX_INIT_STRUCTURE(encoderInputPort);
    encoderInputPort.nPortIndex = IMAGE_ENCODER_INPUT_PORT;
    
    error =OMX_GetParameter(encoder, OMX_IndexParamPortDefinition, &encoderInputPort);
    OMX_TRACE(error);
    
    encoderInputPort.format.image = resizerOutputPort.format.image;
    
    if (settings.colorFormat == GL_RGB) 
    {
        encoderInputPort.format.image.eColorFormat = OMX_COLOR_Format24bitBGR888;
        
    }
    if (settings.colorFormat == GL_RGBA) 
    {
        encoderInputPort.format.image.eColorFormat = OMX_COLOR_Format32bitABGR8888;
    }
    
        
    error = OMX_SetParameter(encoder,
                             OMX_IndexParamPortDefinition,
                             &encoderInputPort);
    OMX_TRACE(error);
    ofLogVerbose(__chan__) << "encoderInputPort: " << GetPortDefinitionString(encoderInputPort);
    
    OMX_PARAM_PORTDEFINITIONTYPE encoderOutputPort;
    OMX_INIT_STRUCTURE(encoderOutputPort);
    encoderOutputPort.nPortIndex = IMAGE_ENCODER_OUTPUT_PORT;
    
    error = OMX_GetParameter(encoder,
                             OMX_IndexParamPortDefinition,
                             &encoderOutputPort);
    OMX_TRACE(error);
        
    encoderOutputPort.format.image.nFrameWidth  =  encoderInputPort.format.image.nFrameWidth;
    encoderOutputPort.format.image.nFrameHeight =  encoderInputPort.format.image.nFrameHeight;
    encoderOutputPort.format.image.eColorFormat = OMX_COLOR_FormatUnused;
    encoderOutputPort.format.image.eCompressionFormat = codingType;
    
    switch (codingType)
    {
        case OMX_IMAGE_CodingPNG:
        {
            if (settings.colorFormat == GL_RGB) 
            {
                encoderOutputPort.format.image.eColorFormat = OMX_COLOR_Format24bitBGR888;
            }
            if (settings.colorFormat == GL_RGBA) 
            {
                encoderOutputPort.format.image.eColorFormat = OMX_COLOR_Format32bitABGR8888;
            }
            break;
        }
            
        case OMX_IMAGE_CodingJPEG:
        {
            
            OMX_IMAGE_PARAM_QFACTORTYPE compressionConfig;
            OMX_INIT_STRUCTURE(compressionConfig);
            compressionConfig.nPortIndex = IMAGE_ENCODER_OUTPUT_PORT;
            error =OMX_GetParameter(encoder,
                                    OMX_IndexParamQFactor,
                                    &compressionConfig);
            OMX_TRACE(error);
            
            compressionConfig.nQFactor = settings.JPGCompressionLevel;
            
            error =OMX_SetParameter(encoder,
                                    OMX_IndexParamQFactor,
                                    &compressionConfig);
            OMX_TRACE(error); 
            
            break;
        }
            
        case OMX_IMAGE_CodingGIF:
        {
            encoderOutputPort.format.image.eColorFormat = OMX_COLOR_Format8bitPalette;
            break;
        }
            
        default:
        {
            break;
        }
    }
    
    error = OMX_SetParameter(encoder,
                            OMX_IndexParamPortDefinition,
                            &encoderOutputPort);
    OMX_TRACE(error);

    ofLogVerbose(__chan__) << "encoderOutputPort: " << GetPortDefinitionString(encoderOutputPort); 
    
    //Set resizer to Idle
    error = OMX_SendCommand(resizer,
                            OMX_CommandStateSet,
                            OMX_StateIdle,
                            NULL);
    OMX_TRACE(error);
    
    error = OMX_SendCommand(encoder,
                            OMX_CommandStateSet,
                            OMX_StateIdle,
                            NULL);
    OMX_TRACE(error);
    
    //Allocate buffers
    
    EnablePortBuffers(encoder,
                      &inputBuffer,
                      IMAGE_ENCODER_INPUT_PORT);
    
    EnablePortBuffers(resizer,
                      &resizeInputBuffer,
                      RESIZER_INPUT_PORT);
    
    EnablePortBuffers(resizer,
                      &resizeOutputBuffer,
                      RESIZER_OUTPUT_PORT);
    
    EnablePortBuffers(encoder,
                      &outputBuffer,
                      IMAGE_ENCODER_OUTPUT_PORT);
    PORT_CHECK();
    
    error = OMX_SendCommand(resizer,
                            OMX_CommandStateSet,
                            OMX_StateExecuting,
                            NULL);
    OMX_TRACE(error);
    
    error = OMX_SendCommand(encoder,
                            OMX_CommandStateSet,
                            OMX_StateExecuting,
                            NULL);
    OMX_TRACE(error);
    
    available = true;

}




void ofxOMXImageEncoder::resetValues()
{
    encoder = NULL;
    inputBuffer = NULL;
    outputBuffer = NULL;
    resizeInputBuffer = NULL;
    resizeOutputBuffer = NULL;

    pixelSize = 0;
    startTime = 0;
    available = false;
    
    savedFiles.clear();
}



#pragma mark PROCESS
void ofxOMXImageEncoder::encode(string filePath_, unsigned char* pixels)
{
    
    ofLogVerbose(__chan__) << "available: " << available;
    available = false;
    startTime = ofGetElapsedTimeMillis();
    filePath = filePath_;
    resizeInputBuffer->pBuffer = pixels;
    resizeInputBuffer->nFilledLen = pixelSize;
    
    //PORT_CHECK();
    OMX_ERRORTYPE error;
    
    
    error = OMX_EmptyThisBuffer(resizer, resizeInputBuffer);
    OMX_TRACE(error);
    //PORT_CHECK();
    
    error = OMX_FillThisBuffer(resizer, resizeOutputBuffer);
    OMX_TRACE(error);
    
}


#pragma mark CALLBACKS

OMX_ERRORTYPE
ofxOMXImageEncoder::resizerFillBufferDone(OMX_HANDLETYPE hComponent,
                                          OMX_PTR pAppData,
                                          OMX_BUFFERHEADERTYPE* pBuffer)
{
    ofLogVerbose(__chan__) << "";
    ofxOMXImageEncoder *imageDecoder = static_cast<ofxOMXImageEncoder*>(pAppData);
    imageDecoder->onResizerFillBuffer();
    return OMX_ErrorNone;
}




void ofxOMXImageEncoder::onResizerFillBuffer()
{
    ofLogVerbose(__chan__) << "";
    
    OMX_ERRORTYPE error;
    
    inputBuffer->pBuffer = resizeOutputBuffer->pBuffer;
    inputBuffer->nFilledLen = resizeOutputBuffer->nFilledLen;
    
    error = OMX_EmptyThisBuffer(encoder, inputBuffer);
    OMX_TRACE(error);
    
    error = OMX_FillThisBuffer(encoder, outputBuffer);
    OMX_TRACE(error);
    
    PORT_CHECK();
    
}

OMX_ERRORTYPE 
ofxOMXImageEncoder::encoderFillBufferDone(OMX_HANDLETYPE hComponent, 
                                          OMX_PTR pAppData, 
                                          OMX_BUFFERHEADERTYPE* pBuffer)
{	
    ofxOMXImageEncoder *imageDecoder = static_cast<ofxOMXImageEncoder*>(pAppData);
    imageDecoder->onEncoderFillBuffer();
    return OMX_ErrorNone;
}

void ofxOMXImageEncoder::onEncoderFillBuffer()
{
    ofLogVerbose(__chan__) << "";
    fileBuffer.append((const char*) outputBuffer->pBuffer + outputBuffer->nOffset,
                      outputBuffer->nFilledLen);
    
    if (settings.enablePrettyFileName) 
    {
        filePath+="_";
        filePath+=settings.getPrettyFileName();
    }
    string fileExtension = settings.getFileExtension();
    if (!ofIsStringInString(filePath, fileExtension)) 
    {
        filePath+=fileExtension;
        
    }
    
    if(fileBuffer.size() > 0)
    {
        bool didWriteFile = ofBufferToFile(filePath, fileBuffer, true);
        if(didWriteFile)
        {
            ofLogVerbose() << filePath << " SUCCESSFULLY WRITTEN IN " << ofGetElapsedTimeMillis() - startTime << " MS";
            savedFiles.push_back(ofFile(filePath));
        }else
        {
            ofLogError(__chan__) << filePath << " COULD NOT BE WRITTEN";
        }
    }else
    {
        ofLogError(__chan__) << "SKIPPING EMPTY FILE: " << filePath;

    }
    
    fileBuffer.clear();
    startTime = 0;
    //PORT_CHECK(); 
    available = true;
}


OMX_ERRORTYPE 
ofxOMXImageEncoder::resizerEventHandlerCallback(OMX_HANDLETYPE hComponent,
                                                OMX_PTR pAppData,
                                                OMX_EVENTTYPE event,
                                                OMX_U32 nData1,
                                                OMX_U32 nData2,
                                                OMX_PTR pEventData)
{

#if defined(DEVEL_MODE)
    ofxOMXImageEncoder *imageDecoder = static_cast<ofxOMXImageEncoder*>(pAppData);
    ofLogVerbose(__chan__) << GetEventString(event);
    if(event == OMX_EventError)
    {
        ofLogVerbose(__chan__) << GetOMXErrorString((OMX_ERRORTYPE) nData1);
        
    }
    if (event == OMX_EventPortSettingsChanged) 
    {
        imageDecoder->onResizerPortSettingsChanged();
    }
#endif
    return OMX_ErrorNone;
}

OMX_ERRORTYPE 
ofxOMXImageEncoder::encoderEventHandlerCallback(OMX_HANDLETYPE hComponent,
                                            OMX_PTR pAppData,
                                            OMX_EVENTTYPE event,
                                            OMX_U32 nData1,
                                            OMX_U32 nData2,
                                            OMX_PTR pEventData)
{
#if defined(DEVEL_MODE)
    ofxOMXImageEncoder *imageDecoder = static_cast<ofxOMXImageEncoder*>(pAppData);
    
    ofLogVerbose(__chan__) << GetEventString(event);
    
    if(event == OMX_EventError)
    {
        ofLogVerbose(__chan__) << GetOMXErrorString((OMX_ERRORTYPE) nData1);
        
    }

    if (event == OMX_EventPortSettingsChanged) 
    {
        imageDecoder->onEncoderPortSettingsChanged();
    }
#endif
    return OMX_ErrorNone;
};





#pragma mark CLOSE
void ofxOMXImageEncoder::close()
{
    OMX_ERRORTYPE error = OMX_SendCommand(encoder,
                                          OMX_CommandStateSet,
                                          OMX_StateIdle,
                                          NULL);
    OMX_TRACE(error);
    teardown();
    
}

ofxOMXImageEncoder::~ofxOMXImageEncoder()
{
    teardown();
    //OMX_Deinit();
}


void ofxOMXImageEncoder::teardown()
{
    if (resizer) 
    {
        OMX_ERRORTYPE error = OMX_ErrorNone;
        
        FlushOMXComponent(resizer, RESIZER_INPUT_PORT);
        FlushOMXComponent(resizer, RESIZER_OUTPUT_PORT);
        
        error = OMX_SendCommand(resizer,
                                OMX_CommandStateSet,
                                OMX_StateIdle,
                                NULL);
        OMX_TRACE(error);
        
        error = OMX_SendCommand(resizer,
                                OMX_CommandStateSet,
                                OMX_StateLoaded,
                                NULL);
        OMX_TRACE(error);
        
        
        error = DisableAllPortsForComponent(&resizer);
        OMX_TRACE(error);
        
        error = OMX_FreeBuffer(resizer,
                               RESIZER_INPUT_PORT,
                               resizeInputBuffer);
        OMX_TRACE(error);
        resizeInputBuffer = NULL;
        
        error = OMX_FreeBuffer(resizer,
                               RESIZER_OUTPUT_PORT,
                               resizeOutputBuffer);
        OMX_TRACE(error);
        resizeOutputBuffer = NULL;
        
        error = OMX_FreeHandle(resizer);
        OMX_TRACE(error);
    }
    
    if (encoder) 
    {
        OMX_ERRORTYPE error = OMX_ErrorNone;
        
        FlushOMXComponent(encoder, IMAGE_ENCODER_INPUT_PORT);
        FlushOMXComponent(encoder, IMAGE_ENCODER_OUTPUT_PORT);
        
        error = OMX_SendCommand(encoder,
                                OMX_CommandStateSet,
                                OMX_StateIdle,
                                NULL);
        OMX_TRACE(error);
        
        error = OMX_SendCommand(encoder,
                                OMX_CommandStateSet,
                                OMX_StateLoaded,
                                NULL);
        OMX_TRACE(error);
        
        
        error = DisableAllPortsForComponent(&encoder);
        OMX_TRACE(error);
        
        error = OMX_FreeBuffer(encoder,
                               IMAGE_ENCODER_INPUT_PORT,
                               inputBuffer);
        OMX_TRACE(error);
        inputBuffer = NULL;
        
        error = OMX_FreeBuffer(encoder,
                               IMAGE_ENCODER_OUTPUT_PORT,
                               outputBuffer);
        OMX_TRACE(error);
        outputBuffer = NULL;
        
        error = OMX_FreeHandle(encoder);
        OMX_TRACE(error);
    }
    resetValues();
}


#pragma mark DEBUG
void ofxOMXImageEncoder::checkPorts(bool doBuffers)
{
    OMX_PARAM_PORTDEFINITIONTYPE input;
    OMX_INIT_STRUCTURE(input);
    input.nPortIndex = IMAGE_ENCODER_INPUT_PORT;
    OMX_ERRORTYPE error =OMX_GetParameter(encoder, OMX_IndexParamPortDefinition, &input);
    OMX_TRACE(error);
    
    
    OMX_PARAM_PORTDEFINITIONTYPE output;
    OMX_INIT_STRUCTURE(output);
    output.nPortIndex = IMAGE_ENCODER_OUTPUT_PORT;
    error =OMX_GetParameter(encoder, OMX_IndexParamPortDefinition, &output);
    OMX_TRACE(error);
    
    
    OMX_PARAM_PORTDEFINITIONTYPE resizerInput;
    OMX_INIT_STRUCTURE(resizerInput);
    resizerInput.nPortIndex = RESIZER_INPUT_PORT;
    error =OMX_GetParameter(resizer, OMX_IndexParamPortDefinition, &resizerInput);
    OMX_TRACE(error);
    
    OMX_PARAM_PORTDEFINITIONTYPE resizerOutput;
    OMX_INIT_STRUCTURE(resizerOutput);
    resizerOutput.nPortIndex = RESIZER_OUTPUT_PORT;
    error =OMX_GetParameter(resizer, OMX_IndexParamPortDefinition, &resizerOutput);
    OMX_TRACE(error);
    
    if (doBuffers) 
    {
        ofLogVerbose(__chan__) << "resizeInputBuffer: " << GetBufferHeaderString(resizeInputBuffer);
        ofLogVerbose(__chan__) << "resizeOutputBuffer: " << GetBufferHeaderString(resizeOutputBuffer);
        ofLogVerbose(__chan__) << "inputBuffer: " << GetBufferHeaderString(inputBuffer);
        ofLogVerbose(__chan__) << "outputBuffer: " << GetBufferHeaderString(outputBuffer);
    }
    ofLogVerbose(__chan__) << "resizerInput: " << GetPortDefinitionString(resizerInput);
    ofLogVerbose(__chan__) << "resizerOutput: " << GetPortDefinitionString(resizerOutput);
    ofLogVerbose(__chan__) << "encoderInputPortDefinition: " << GetPortDefinitionString(input);
    ofLogVerbose(__chan__) << "encoderOutputPort: " << GetPortDefinitionString(output);
 
}

void ofxOMXImageEncoder::probeEncoder()
{
    OMX_ERRORTYPE error = OMX_ErrorNone;
    
    OMX_PARAM_PORTDEFINITIONTYPE encoderOutputPort;
    OMX_INIT_STRUCTURE(encoderOutputPort);
    encoderOutputPort.nPortIndex = IMAGE_ENCODER_OUTPUT_PORT;
    
    error =OMX_GetParameter(encoder, OMX_IndexParamPortDefinition, &encoderOutputPort);
    OMX_TRACE(error);
    
    vector<OMX_IMAGE_CODINGTYPE> workingCodeTypes;
    
    workingCodeTypes.push_back(OMX_IMAGE_CodingBMP);
    workingCodeTypes.push_back(OMX_IMAGE_CodingGIF);
    workingCodeTypes.push_back(OMX_IMAGE_CodingPPM);
    workingCodeTypes.push_back(OMX_IMAGE_CodingTGA);
    workingCodeTypes.push_back(OMX_IMAGE_CodingJPEG);
    workingCodeTypes.push_back(OMX_IMAGE_CodingPNG);
    
    for (size_t i=0; i<workingCodeTypes.size(); i++)
    {
        encoderOutputPort.format.image.eColorFormat = OMX_COLOR_FormatUnused;
        encoderOutputPort.format.image.eCompressionFormat = workingCodeTypes[i];
        error =OMX_SetParameter(encoder, OMX_IndexParamPortDefinition, &encoderOutputPort);
        OMX_TRACE(error);
        if (error == OMX_ErrorNone) 
        {
            ofLogVerbose() << GetImageCodingString(workingCodeTypes[i]);
            ProbeImageColorFormats(encoder, encoderOutputPort);
        }
    }
}

