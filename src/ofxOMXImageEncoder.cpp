#include "ofxOMXImageEncoder.h"




ofxOMXImageEncoder::ofxOMXImageEncoder()
{
    resetValues();
}



#pragma mark SETUP
void ofxOMXImageEncoder::setup(ofxOMXImageEncoderSettings settings_)
{
    settings = settings_;
    OMX_ERRORTYPE error = OMX_ErrorNone;
    error = OMX_Init();
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
    inputPortDefinition.format.image.nStride        =   inputPortDefinition.format.image.nFrameWidth;
    //inputPortDefinition.format.video.eColorFormat = OMX_COLOR_FormatYUV420PackedPlanar;
    if (settings.colorFormat == GL_RGB) 
    {
        pixelSize = settings.width * settings.height * 3;
        inputPortDefinition.format.image.eColorFormat = OMX_COLOR_Format24bitBGR888;
    }
    if (settings.colorFormat == GL_RGBA) 
    {
        pixelSize = settings.width * settings.height * 4;
        inputPortDefinition.format.image.eColorFormat = OMX_COLOR_Format32bitABGR8888;
    }

    error =OMX_SetParameter(encoder, OMX_IndexParamPortDefinition, &inputPortDefinition);
    OMX_TRACE(error);
    
    OMX_PARAM_PORTDEFINITIONTYPE encoderOutputPortDefinition;
    OMX_INIT_STRUCTURE(encoderOutputPortDefinition);
    encoderOutputPortDefinition.nPortIndex = IMAGE_ENCODER_OUTPUT_PORT;
    
    error =OMX_GetParameter(encoder, OMX_IndexParamPortDefinition, &encoderOutputPortDefinition);
    OMX_TRACE(error);
    
    ofLogVerbose(__func__) << "encoderOutputPortDefinition: " << GetPortDefinitionString(encoderOutputPortDefinition); 
    
    encoderOutputPortDefinition.format.image.nFrameWidth    =   settings.width;
    encoderOutputPortDefinition.format.image.nFrameHeight   =   settings.height;
    encoderOutputPortDefinition.format.image.nSliceHeight   =   encoderOutputPortDefinition.format.image.nFrameHeight;
    encoderOutputPortDefinition.format.image.nStride        =   encoderOutputPortDefinition.format.image.nFrameWidth;
    //has to be set first or will automatically go to GIF?
    encoderOutputPortDefinition.format.image.eColorFormat       = OMX_COLOR_FormatUnused;
    encoderOutputPortDefinition.format.image.eCompressionFormat = (OMX_IMAGE_CODINGTYPE)settings.imageType;
    
    error =OMX_SetParameter(encoder, OMX_IndexParamPortDefinition, &encoderOutputPortDefinition);
    OMX_TRACE(error);
    
    switch (encoderOutputPortDefinition.format.image.eCompressionFormat)
    {
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
        default:
        {
            error =OMX_SetParameter(encoder, OMX_IndexParamPortDefinition, &encoderOutputPortDefinition);
            OMX_TRACE(error);
            break;
        }
    }

    

 
    /*
    OMX_IMAGE_CodingPNG
        24bitBGR888
        32bitABGR8888
        32bitARGB8888
        Unused
    */
    /*
     OMX_IMAGE_CodingBMP
    OMX_IMAGE_CodingGIF
    OMX_IMAGE_CodingPPM
    OMX_IMAGE_CodingTGA
    OMX_IMAGE_CodingJPEG
    OMX_IMAGE_CodingPNG
    
    */
    //Allocate buffers
    //Set encoder to Idle
    error = OMX_SendCommand(encoder, OMX_CommandStateSet, OMX_StateIdle, NULL);
    OMX_TRACE(error);
    
   
    EnablePortBuffers(encoder, &inputBuffer, IMAGE_ENCODER_INPUT_PORT);
    EnablePortBuffers(encoder, &outputBuffer, IMAGE_ENCODER_OUTPUT_PORT);

    //checkPorts();
    
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
        //imageDecoder->onEncoderPortSettingsChanged();
    }
    return OMX_ErrorNone;
};

void ofxOMXImageEncoder::onEncoderPortSettingsChanged()
{
    //ofLogVerbose(__func__) << "";
}

OMX_ERRORTYPE 
ofxOMXImageEncoder::encoderEmptyBufferDone(OMX_HANDLETYPE hComponent, 
                                                     OMX_PTR pAppData, 
                                                     OMX_BUFFERHEADERTYPE* pBuffer)
{
    
    //never called for some reason
    //ofLogVerbose(__func__) << "";
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
    //ofLogVerbose(__func__) << "";
    ofxOMXImageEncoder *imageDecoder = static_cast<ofxOMXImageEncoder*>(pAppData);
    imageDecoder->onEncoderFillBuffer();
    return OMX_ErrorNone;
}

#pragma mark DEBUG
void ofxOMXImageEncoder::checkPorts()
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
    
    
    ofLogVerbose(__func__) << "inputBuffer: " << GetBufferHeaderString(inputBuffer);
    ofLogVerbose(__func__) << "inputPortDefinition: " << GetPortDefinitionString(input);
    
    ofLogVerbose(__func__) << "outputBuffer: " << GetBufferHeaderString(outputBuffer);
    ofLogVerbose(__func__) << "encoderOutputPortDefinition: " << GetPortDefinitionString(output);  
}
