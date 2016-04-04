#include "ofxOMXImageEncoder.h"

#define LOG_LINE ofLogVerbose(__func__) << __LINE__ << endl;

#define PORT_CHECK_NO_BUFFER(...) LOG_LINE checkPorts(false); 
#define PORT_CHECK(...) LOG_LINE checkPorts(); 





ofxOMXImageEncoder::ofxOMXImageEncoder()
{
    workingCodeTypes.push_back(OMX_IMAGE_CodingBMP);
    workingCodeTypes.push_back(OMX_IMAGE_CodingGIF);
    workingCodeTypes.push_back(OMX_IMAGE_CodingPPM);
    workingCodeTypes.push_back(OMX_IMAGE_CodingTGA);
    workingCodeTypes.push_back(OMX_IMAGE_CodingJPEG);
    workingCodeTypes.push_back(OMX_IMAGE_CodingPNG);
    resetValues();
}

void ofxOMXImageEncoder::probeEncoder()
{
    OMX_ERRORTYPE error = OMX_ErrorNone;

    OMX_PARAM_PORTDEFINITIONTYPE encoderOutputPortDefinition;
    OMX_INIT_STRUCTURE(encoderOutputPortDefinition);
    encoderOutputPortDefinition.nPortIndex = IMAGE_ENCODER_OUTPUT_PORT;
    
    error =OMX_GetParameter(encoder, OMX_IndexParamPortDefinition, &encoderOutputPortDefinition);
    OMX_TRACE(error);
    
    for (size_t i=0; i<workingCodeTypes.size(); i++)
    {
        encoderOutputPortDefinition.format.image.eColorFormat = OMX_COLOR_FormatUnused;
        encoderOutputPortDefinition.format.image.eCompressionFormat = workingCodeTypes[i];
        error =OMX_SetParameter(encoder, OMX_IndexParamPortDefinition, &encoderOutputPortDefinition);
         OMX_TRACE(error);
        if (error == OMX_ErrorNone) 
        {
            ofLogVerbose() << GetImageCodingString(workingCodeTypes[i]);
            ProbeImageColorFormats(encoder, encoderOutputPortDefinition);
        }
    }
}

void printResizeInfo(OMX_PARAM_RESIZETYPE resizeConfig)
{
    stringstream ss;
    ss << endl;
    switch (resizeConfig.eMode) 
    {
        case OMX_RESIZE_NONE:   ss << "eMode: " << "OMX_RESIZE_NONE"    << endl; break;
        case OMX_RESIZE_CROP:   ss << "eMode: " << "OMX_RESIZE_CROP"    << endl; break;
        case OMX_RESIZE_BOX:    ss << "eMode: " << "OMX_RESIZE_BOX"     << endl; break;
        case OMX_RESIZE_BYTES:  ss << "eMode: " << "OMX_RESIZE_BYTES"   << endl; break;
            
    }
    ss << "nMaxWidth: " << resizeConfig.nMaxWidth << endl;
    ss << "nMaxHeight: " << resizeConfig.nMaxHeight << endl;
    ss << "nMaxBytes: " << resizeConfig.nMaxBytes << endl;
    ss << "bPreserveAspectRatio: " << resizeConfig.bPreserveAspectRatio << endl;
    ss << "bAllowUpscaling: " << resizeConfig.bAllowUpscaling << endl;
    
    ofLogVerbose(__func__) << "resizeConfig: " << ss.str();
    
}
void ofxOMXImageEncoder::setupWithResizer()
{
    
    OMX_ERRORTYPE error = OMX_ErrorNone;

    
    OMX_CALLBACKTYPE resizerCallbacks;
    resizerCallbacks.EventHandler		= &ofxOMXImageEncoder::resizerEventHandlerCallback;
    resizerCallbacks.EmptyBufferDone	= &ofxOMXImageEncoder::resizerEmptyBufferDone;
    resizerCallbacks.FillBufferDone		= &ofxOMXImageEncoder::resizerFillBufferDone;
    
    error =OMX_GetHandle(&resizer, OMX_RESIZER, this, &resizerCallbacks);
    OMX_TRACE(error);
    
   
    
    error = DisableAllPortsForComponent(&resizer);
    OMX_TRACE(error);
    
    
    OMX_PARAM_PORTDEFINITIONTYPE inputPort;
    OMX_INIT_STRUCTURE(inputPort);
    inputPort.nPortIndex = RESIZER_INPUT_PORT;
    
    error =OMX_GetParameter(resizer, OMX_IndexParamPortDefinition, &inputPort);
    OMX_TRACE(error);
    
    ofLogVerbose(__func__) << "inputPort: " << GetPortDefinitionString(inputPort); 
    
    
    int numColors = 0;
    if (settings.colorFormat == GL_RGB) 
    {
        numColors = 3;
        inputPort.format.image.eColorFormat = OMX_COLOR_Format24bitBGR888;
        
    }
    if (settings.colorFormat == GL_RGBA) 
    {
        numColors = 4;
        inputPort.format.image.eColorFormat = OMX_COLOR_Format32bitABGR8888;
    }
    
    
    inputPort.format.image.nFrameWidth  =   settings.width;
    inputPort.format.image.nFrameHeight =   settings.height;
    inputPort.format.image.nSliceHeight =   settings.height;
    inputPort.format.image.nStride      =   settings.width*numColors;

   
    
    
    //Stride is byte-per-pixel*width
    //See mmal/util/mmal_util.c, mmal_encoding_width_to_stride()
    int stride = settings.width * numColors;    
    inputPort.format.image.nStride = stride;
    
    
    error =OMX_SetParameter(resizer, OMX_IndexParamPortDefinition, &inputPort);
    OMX_TRACE(error);
    
    ofLogVerbose(__func__) << "inputPort: " << GetPortDefinitionString(inputPort); 
    

    
    OMX_PARAM_PORTDEFINITIONTYPE outputPort;
    OMX_INIT_STRUCTURE(outputPort);
    outputPort.nPortIndex = RESIZER_OUTPUT_PORT;
     
     
    error =OMX_GetParameter(resizer, OMX_IndexParamPortDefinition, &outputPort);
    ofLogVerbose(__func__) << "outputPort: " << GetPortDefinitionString(outputPort); 
     
  
     
    outputPort.format.image.nFrameWidth  =  settings.outputWidth;
    outputPort.format.image.nFrameHeight =  settings.outputHeight;
    //outputPort.format.image.nSliceHeight =  settings.outputHeight;
    //outputPort.format.image.nStride      =  settings.outputWidth*numColors;
 
    error =OMX_SetParameter(resizer, OMX_IndexParamPortDefinition, &outputPort);
    OMX_TRACE(error);
    ofLogVerbose(__func__) << "outputPort: " << GetPortDefinitionString(outputPort); 
     
#if 0
    OMX_PARAM_RESIZETYPE resizeConfig;
    OMX_INIT_STRUCTURE(resizeConfig);
    resizeConfig.nPortIndex = RESIZER_OUTPUT_PORT;
    
    error =OMX_GetParameter(resizer, OMX_IndexParamResize, &resizeConfig);
    OMX_TRACE(error);
    
    printResizeInfo(resizeConfig);
    
    //resizeConfig.eMode = OMX_RESIZE_BOX;
    resizeConfig.nMaxWidth = settings.outputWidth;
    resizeConfig.nMaxHeight = settings.outputHeight;
    //resizeConfig.nMaxBytes = 0;
    resizeConfig.bPreserveAspectRatio = OMX_FALSE;
    resizeConfig.bAllowUpscaling = OMX_FALSE;
    
    error =OMX_SetParameter(resizer, OMX_IndexParamResize, &resizeConfig);
    OMX_TRACE(error);
    
    printResizeInfo(resizeConfig);
    
    
    
    OMX_CONFIG_RECTTYPE cropConfig;
    OMX_INIT_STRUCTURE(cropConfig);
    cropConfig.nPortIndex = RESIZER_OUTPUT_PORT;
    
    
    error =OMX_GetParameter(resizer, OMX_IndexConfigCommonOutputCrop, &cropConfig);
    OMX_TRACE(error);
    
    stringstream ss;
    ss << endl;
    ss << "nSize: " << cropConfig.nSize << endl;
    ss << "nPortIndex: " << cropConfig.nPortIndex << endl;
    ss << "nLeft: " << cropConfig.nLeft << endl;
    ss << "nTop: " << cropConfig.nTop << endl;
    ss << "nWidth: " << cropConfig.nWidth << endl;
    ss << "nHeight: " << cropConfig.nHeight << endl;
    
    ofLogVerbose(__func__) << "cropConfig: " << ss.str();
    
#endif
    //Set resizer to Idle
    error = OMX_SendCommand(resizer, OMX_CommandStateSet, OMX_StateIdle, NULL);
    OMX_TRACE(error);
    
 
    
    OMX_CALLBACKTYPE encoderCallbacks;
    encoderCallbacks.EventHandler		= &ofxOMXImageEncoder::encoderEventHandlerCallback;
    encoderCallbacks.EmptyBufferDone	= &ofxOMXImageEncoder::encoderEmptyBufferDone;
    encoderCallbacks.FillBufferDone		= &ofxOMXImageEncoder::encoderFillBufferDone;
    
    error =OMX_GetHandle(&encoder, OMX_IMAGE_ENCODER, this , &encoderCallbacks);
    OMX_TRACE(error);
    
    error = DisableAllPortsForComponent(&encoder);
    OMX_TRACE(error);
    
    OMX_PARAM_PORTDEFINITIONTYPE inputPortDefinition;
    OMX_INIT_STRUCTURE(inputPortDefinition);
    inputPortDefinition.nPortIndex = IMAGE_ENCODER_INPUT_PORT;
    
    error =OMX_GetParameter(encoder, OMX_IndexParamPortDefinition, &inputPortDefinition);
    OMX_TRACE(error);
    
    inputPortDefinition.format.image.nFrameWidth    =   settings.width;
    inputPortDefinition.format.image.nFrameHeight   =   settings.height;
    inputPortDefinition.format.image.nSliceHeight   =   inputPortDefinition.format.image.nFrameHeight;
    if (settings.colorFormat == GL_RGB) 
    {
        inputPortDefinition.format.image.eColorFormat = OMX_COLOR_Format24bitBGR888;
        
    }
    if (settings.colorFormat == GL_RGBA) 
    {
        inputPortDefinition.format.image.eColorFormat = OMX_COLOR_Format32bitABGR8888;
    }
    
    inputPortDefinition.format.image.nStride = stride;
    
    
    error =OMX_SetParameter(encoder, OMX_IndexParamPortDefinition, &inputPortDefinition);
    OMX_TRACE(error);
    
    
    OMX_PARAM_PORTDEFINITIONTYPE encoderOutputPortDefinition;
    OMX_INIT_STRUCTURE(encoderOutputPortDefinition);
    encoderOutputPortDefinition.nPortIndex = IMAGE_ENCODER_OUTPUT_PORT;
    
    error =OMX_GetParameter(encoder, OMX_IndexParamPortDefinition, &encoderOutputPortDefinition);
    OMX_TRACE(error);
    
    ofLogVerbose(__func__) << "encoderOutputPortDefinition: " << GetPortDefinitionString(encoderOutputPortDefinition); 
    
    
    encoderOutputPortDefinition.format.image.bFlagErrorConcealment = OMX_FALSE;
    
    
    
    //has to be set OMX_COLOR_FormatUnused first or will automatically go to GIF/8bit?
    encoderOutputPortDefinition.format.image.eColorFormat = OMX_COLOR_FormatUnused;
    encoderOutputPortDefinition.format.image.eCompressionFormat = codingType;
    
    error =OMX_SetParameter(encoder, OMX_IndexParamPortDefinition, &encoderOutputPortDefinition);
    OMX_TRACE(error);

    
    error = OMX_SetupTunnel(resizer, RESIZER_OUTPUT_PORT,
                            encoder, IMAGE_ENCODER_INPUT_PORT);
    OMX_TRACE(error);
    
    
    //Allocate buffers
    //EnablePortBuffers(resizer, &inputBuffer, RESIZER_INPUT_PORT);
    //EnablePortBuffers(resizer, &outputBuffer, RESIZER_OUTPUT_PORT);
    

}
void ofxOMXImageEncoder::onResizerPortSettingsChanged()
{
 
}

#pragma mark SETUP
void ofxOMXImageEncoder::setup(ofxOMXImageEncoderSettings settings_)
{
    settings = settings_;
    settings.validate();
    OMX_ERRORTYPE error = OMX_ErrorNone;
    error = OMX_Init();
    OMX_TRACE(error);
    
    
    codingType = (OMX_IMAGE_CODINGTYPE)settings.imageType;
    
    bool needResize = false;
    //OMX_RESIZER
    if ((settings.outputWidth != settings.width) || (settings.height != settings.outputHeight))
    {
        setupWithResizer();
        return;
    }
    

/*
    OMX_IMAGE_CodingBMP
    OMX_IMAGE_CodingGIF
    OMX_IMAGE_CodingPPM
    OMX_IMAGE_CodingTGA
    OMX_IMAGE_CodingJPEG
    OMX_IMAGE_CodingPNG
*/ 
    
    
    OMX_CALLBACKTYPE encoderCallbacks;
    encoderCallbacks.EventHandler		= &ofxOMXImageEncoder::encoderEventHandlerCallback;
    encoderCallbacks.EmptyBufferDone	= &ofxOMXImageEncoder::encoderEmptyBufferDone;
    encoderCallbacks.FillBufferDone		= &ofxOMXImageEncoder::encoderFillBufferDone;
    
    error =OMX_GetHandle(&encoder, OMX_IMAGE_ENCODER, this , &encoderCallbacks);
    OMX_TRACE(error);
    
    error = DisableAllPortsForComponent(&encoder);
    OMX_TRACE(error);

    OMX_PARAM_PORTDEFINITIONTYPE inputPortDefinition;
    OMX_INIT_STRUCTURE(inputPortDefinition);
    inputPortDefinition.nPortIndex = IMAGE_ENCODER_INPUT_PORT;
    
    error =OMX_GetParameter(encoder, OMX_IndexParamPortDefinition, &inputPortDefinition);
    OMX_TRACE(error);
    
    inputPortDefinition.format.image.nFrameWidth    =   settings.width;
    inputPortDefinition.format.image.nFrameHeight   =   settings.height;
    inputPortDefinition.format.image.nSliceHeight   =   inputPortDefinition.format.image.nFrameHeight;
 
    
    int numColors = 0;
    if (settings.colorFormat == GL_RGB) 
    {
        numColors = 3;
        inputPortDefinition.format.image.eColorFormat = OMX_COLOR_Format24bitBGR888;

    }
    if (settings.colorFormat == GL_RGBA) 
    {
        numColors = 4;
        inputPortDefinition.format.image.eColorFormat = OMX_COLOR_Format32bitABGR8888;
    }
    
    pixelSize = settings.width * settings.height * numColors;
    
    //Stride is byte-per-pixel*width
    //See mmal/util/mmal_util.c, mmal_encoding_width_to_stride()
    int stride = settings.width * numColors;    
    inputPortDefinition.format.image.nStride = stride;
    
    
    error =OMX_SetParameter(encoder, OMX_IndexParamPortDefinition, &inputPortDefinition);
    OMX_TRACE(error);
    
        
    
    OMX_PARAM_PORTDEFINITIONTYPE encoderOutputPortDefinition;
    OMX_INIT_STRUCTURE(encoderOutputPortDefinition);
    encoderOutputPortDefinition.nPortIndex = IMAGE_ENCODER_OUTPUT_PORT;
    
    error =OMX_GetParameter(encoder, OMX_IndexParamPortDefinition, &encoderOutputPortDefinition);
    OMX_TRACE(error);
    
    ofLogVerbose(__func__) << "encoderOutputPortDefinition: " << GetPortDefinitionString(encoderOutputPortDefinition); 
    
    
    encoderOutputPortDefinition.format.image.bFlagErrorConcealment = OMX_FALSE;

    

    //has to be set OMX_COLOR_FormatUnused first or will automatically go to GIF/8bit?
    encoderOutputPortDefinition.format.image.eColorFormat = OMX_COLOR_FormatUnused;
    encoderOutputPortDefinition.format.image.eCompressionFormat = codingType;
    
    error =OMX_SetParameter(encoder, OMX_IndexParamPortDefinition, &encoderOutputPortDefinition);
    OMX_TRACE(error);
    
    //probeEncoder();
    
    switch (codingType)
    {
/*
        BMP
        24bitBGR888
        
        GIF
        8bitPalette
        
        PPM
        24bitBGR888
        
        TGA
        24bitBGR888
        32bitABGR8888
        32bitARGB8888
        
        JPEG
        CbYCrY
        CrYCbY
        YCbYCr
        YCrYCb
        YUV420PackedPlanar
        YUV422PackedPlanar
        
        PNG
        24bitBGR888
        32bitABGR8888
        32bitARGB8888 
*/
        case OMX_IMAGE_CodingPNG:
        {
            if (settings.colorFormat == GL_RGB) 
            {
                encoderOutputPortDefinition.format.image.eColorFormat = OMX_COLOR_Format24bitBGR888;
            }
            if (settings.colorFormat == GL_RGBA) 
            {
                encoderOutputPortDefinition.format.image.eColorFormat = OMX_COLOR_Format32bitABGR8888;
            }
            error =OMX_SetParameter(encoder, OMX_IndexParamPortDefinition, &encoderOutputPortDefinition);
            OMX_TRACE(error);
            
            break;
        }
        case OMX_IMAGE_CodingJPEG:
        {
            error =OMX_SetParameter(encoder, OMX_IndexParamPortDefinition, &encoderOutputPortDefinition);
            OMX_TRACE(error);
            
            OMX_IMAGE_PARAM_QFACTORTYPE compressionConfig;
            OMX_INIT_STRUCTURE(compressionConfig);
            compressionConfig.nPortIndex = IMAGE_ENCODER_OUTPUT_PORT;
            error =OMX_GetParameter(encoder, OMX_IndexParamQFactor, &compressionConfig);
            OMX_TRACE(error);
            
            compressionConfig.nQFactor = settings.JPGCompressionLevel;
            
            error =OMX_SetParameter(encoder, OMX_IndexParamQFactor, &compressionConfig);
            OMX_TRACE(error); 
            
            break;
        }
        case OMX_IMAGE_CodingBMP:
        {
            //encoderOutputPortDefinition.format.image.nStride        = settings.outputWidth * 3;
            //encoderOutputPortDefinition.format.image.eColorFormat   = OMX_COLOR_Format24bitBGR888;
            break;
        }
            
        case OMX_IMAGE_CodingGIF:
        {
            encoderOutputPortDefinition.format.image.eColorFormat = OMX_COLOR_Format8bitPalette;
            break;
        }
        case OMX_IMAGE_CodingPPM:
        {
            encoderOutputPortDefinition.format.image.eColorFormat = OMX_COLOR_Format24bitBGR888;
            break;
        }
            
        case OMX_IMAGE_CodingTGA:
        {
            if (settings.colorFormat == GL_RGB) 
            {
                encoderOutputPortDefinition.format.image.eColorFormat = OMX_COLOR_Format24bitBGR888;
            }
            if (settings.colorFormat == GL_RGBA) 
            {
                encoderOutputPortDefinition.format.image.eColorFormat = OMX_COLOR_Format32bitABGR8888;
            }
            break;
        }    
        default:
        {
            error =OMX_SetParameter(encoder, OMX_IndexParamPortDefinition, &encoderOutputPortDefinition);
            OMX_TRACE(error);
            break;
        }
    }

    PORT_CHECK_NO_BUFFER();
    
    //Set encoder to Idle
    error = OMX_SendCommand(encoder, OMX_CommandStateSet, OMX_StateIdle, NULL);

    
    if (resizer) 
    {
        error = OMX_SetupTunnel(encoder, IMAGE_ENCODER_INPUT_PORT,
                                resizer, RESIZER_INPUT_PORT);
        OMX_TRACE(error);
        
        error = OMX_SetupTunnel(resizer, RESIZER_OUTPUT_PORT,
                                encoder, IMAGE_ENCODER_OUTPUT_PORT);
        OMX_TRACE(error);
    }
    //Allocate buffers
    EnablePortBuffers(encoder, &inputBuffer, IMAGE_ENCODER_INPUT_PORT);
    EnablePortBuffers(encoder, &outputBuffer, IMAGE_ENCODER_OUTPUT_PORT);
    
    PORT_CHECK();
    

    error = OMX_SendCommand(encoder, OMX_CommandPortEnable, IMAGE_ENCODER_INPUT_PORT, NULL);
    OMX_TRACE(error);
    
    error = OMX_SendCommand(encoder, OMX_CommandPortEnable, IMAGE_ENCODER_OUTPUT_PORT, NULL);
    OMX_TRACE(error);
    
    OMX_SendCommand(encoder, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    OMX_TRACE(error);
    
    available = true;

}

#pragma mark PROCESS
void ofxOMXImageEncoder::encode(string filePath_, unsigned char* pixels)
{
    
    ofLogVerbose(__func__) << "";
    available = false;
    startTime = ofGetElapsedTimeMillis();
    filePath = filePath_;
	inputBuffer->pBuffer = pixels;
	inputBuffer->nFilledLen = pixelSize;
	
   //checkPorts(); 
    
    OMX_ERRORTYPE error = OMX_EmptyThisBuffer(encoder, inputBuffer);
    OMX_TRACE(error);

    error = OMX_FillThisBuffer(encoder, outputBuffer);
    OMX_TRACE(error);
}


#pragma mark CLOSE
void ofxOMXImageEncoder::close()
{
    OMX_ERRORTYPE error = OMX_SendCommand(encoder, OMX_CommandStateSet, OMX_StateIdle, NULL);
    OMX_TRACE(error);
    teardown();
    
}

void ofxOMXImageEncoder::resetValues()
{
    encoder = NULL;
    inputBuffer = NULL;
    outputBuffer = NULL;
    pixelSize = 0;
    startTime = 0;
    available = false;
}

ofxOMXImageEncoder::~ofxOMXImageEncoder()
{
    teardown();
}


void ofxOMXImageEncoder::teardown()
{
    if (encoder) 
    {
        OMX_ERRORTYPE error = OMX_ErrorNone;
        
        FlushOMXComponent(encoder, IMAGE_ENCODER_INPUT_PORT);
        FlushOMXComponent(encoder, IMAGE_ENCODER_OUTPUT_PORT);
        
        error = OMX_SendCommand(encoder, OMX_CommandStateSet, OMX_StateIdle, NULL);
        OMX_TRACE(error);
        
        error = OMX_SendCommand(encoder, OMX_CommandStateSet, OMX_StateLoaded, NULL);
        OMX_TRACE(error);
        
        
        error = DisableAllPortsForComponent(&encoder);
        OMX_TRACE(error);
        
        error = OMX_FreeBuffer(encoder, IMAGE_ENCODER_INPUT_PORT, inputBuffer);
        OMX_TRACE(error);
        inputBuffer = NULL;
        
        error = OMX_FreeBuffer(encoder, IMAGE_ENCODER_OUTPUT_PORT, outputBuffer);
        OMX_TRACE(error);
        outputBuffer = NULL;
        
        error = OMX_FreeHandle(encoder);
        OMX_TRACE(error);
    }
    resetValues();
}

#pragma mark CALLBACKS
void ofxOMXImageEncoder::onEncoderFillBuffer()
{
    ofLogVerbose(__func__) << "";
    fileBuffer.append((const char*) outputBuffer->pBuffer + outputBuffer->nOffset, outputBuffer->nFilledLen);
    
    ofLogVerbose(__func__) << "outputBuffer->nFilledLen: " << outputBuffer->nFilledLen;
    ofLogVerbose(__func__) << "fileBuffer.size: " << fileBuffer.size();

    
    string fileExtension = settings.getFileExtension();
    if (!ofIsStringInString(filePath, fileExtension)) 
    {
        filePath+=fileExtension;
        
    }
    ofLogVerbose(__func__) << "filePath: " << filePath;
    bool didWriteFile = ofBufferToFile(filePath, fileBuffer, true);
    if(didWriteFile)
    {
        ofLogVerbose(__func__) << filePath << " SUCCESSFULLY WRITTEN IN " << ofGetElapsedTimeMillis() - startTime;
    }else
    {
        ofLogError(__func__) << filePath << " COULD NOT BE WRITTEN";
    }
    fileBuffer.clear();
    startTime = 0;
    //checkPorts(); 
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
    ofxOMXImageEncoder *imageDecoder = static_cast<ofxOMXImageEncoder*>(pAppData);
    
    ofLogVerbose(__func__) << GetEventString(event);
    
    if(event == OMX_EventError)
    {
        ofLogVerbose(__func__) << GetOMXErrorString((OMX_ERRORTYPE) nData1);
        
    }
    if (event == OMX_EventPortSettingsChanged) 
    {
        imageDecoder->onResizerPortSettingsChanged();
    }
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
    ofxOMXImageEncoder *imageDecoder = static_cast<ofxOMXImageEncoder*>(pAppData);
    
    ofLogVerbose(__func__) << GetEventString(event);
    
    if(event == OMX_EventError)
    {
        ofLogVerbose(__func__) << GetOMXErrorString((OMX_ERRORTYPE) nData1);
        
    }
    if (event == OMX_EventPortSettingsChanged) 
    {
        imageDecoder->onEncoderPortSettingsChanged();
    }
    return OMX_ErrorNone;
};

void ofxOMXImageEncoder::onEncoderPortSettingsChanged()
{
    ofLogVerbose(__func__) << "";
}



OMX_ERRORTYPE 
ofxOMXImageEncoder::encoderEmptyBufferDone(OMX_HANDLETYPE hComponent, 
                                                     OMX_PTR pAppData, 
                                                     OMX_BUFFERHEADERTYPE* pBuffer)
{
    
    //never called for some reason
    ofLogVerbose(__func__) << "";
    //ofxOMXImageEncoder *imageDecoder = static_cast<ofxOMXImageEncoder*>(pAppData);
    return OMX_ErrorNone;
    
}

void ofxOMXImageEncoder::onEncoderEmptyBuffer()
{
    ofLogVerbose(__func__) << "";    
}


OMX_ERRORTYPE 
ofxOMXImageEncoder::encoderFillBufferDone(OMX_HANDLETYPE hComponent, 
                                                    OMX_PTR pAppData, 
                                                    OMX_BUFFERHEADERTYPE* pBuffer)
{	
    ofLogVerbose(__func__) << "";
    ofxOMXImageEncoder *imageDecoder = static_cast<ofxOMXImageEncoder*>(pAppData);
    imageDecoder->onEncoderFillBuffer();
    return OMX_ErrorNone;
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
    
    if (doBuffers) 
    {
        ofLogVerbose(__func__) << "inputBuffer: " << GetBufferHeaderString(inputBuffer);

    }
    ofLogVerbose(__func__) << "inputPortDefinition: " << GetPortDefinitionString(input);
    
    if (doBuffers) 
    {
        ofLogVerbose(__func__) << "outputBuffer: " << GetBufferHeaderString(outputBuffer);
    }
    ofLogVerbose(__func__) << "encoderOutputPortDefinition: " << GetPortDefinitionString(output);  
}
