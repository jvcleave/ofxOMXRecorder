#pragma once
#include "OMX_Maps.h"


class ofxOMXImageEncoderSettings
{
public:
    
    enum IMAGE_TYPE
    {
        BMP = OMX_IMAGE_CodingBMP,
        GIF = OMX_IMAGE_CodingGIF,
        PPM = OMX_IMAGE_CodingPPM,
        TGA = OMX_IMAGE_CodingTGA,
        JPG = OMX_IMAGE_CodingJPEG,
        PNG = OMX_IMAGE_CodingPNG
    };
    int width;
    int height;
    int colorFormat;
    float bitrateMegabytesPerSecond;
    bool enablePrettyFileName;
    IMAGE_TYPE imageType;
    int JPGCompressionLevel;
    ofxOMXImageEncoderSettings()
    {
        width = 1280;
        height = 720;
        colorFormat = GL_RGBA;
        bitrateMegabytesPerSecond = 2.0;
        enablePrettyFileName = true;
        imageType = PNG;
        JPGCompressionLevel = 30;
    };
    
    string getFileExtension()
    {
        string fileExt = "UNSET";
        switch (imageType) 
        {
            case BMP: fileExt = ".bmp"; break;
            case GIF: fileExt = ".gif"; break;
            case TGA: fileExt = ".tga"; break;
            case PPM: fileExt = ".ppm"; break;
            case JPG: fileExt = ".jpg"; break;
            case PNG: fileExt = ".png"; break;
        }
        return fileExt;
    }
};


class ofxOMXImageEncoder
{
public:
    ofxOMXImageEncoder();
    ~ofxOMXImageEncoder();
    void setup(ofxOMXImageEncoderSettings);
    void encode(string filePath_, unsigned char* pixels);
    OMX_IMAGE_CODINGTYPE codingType;
    bool isAvailable()
    {
        return available;
    }
    void close();
    
private:
    void checkPorts(bool doBuffers=true);
    void resetValues();
    void teardown();
    ofxOMXImageEncoderSettings settings;
    OMX_HANDLETYPE encoder;

    OMX_BUFFERHEADERTYPE* inputBuffer;
    OMX_BUFFERHEADERTYPE* outputBuffer;

    
    
    static OMX_ERRORTYPE 
    encoderEventHandlerCallback(OMX_HANDLETYPE, 
                                OMX_PTR, 
                                OMX_EVENTTYPE, 
                                OMX_U32, OMX_U32, 
                                OMX_PTR);
    static OMX_ERRORTYPE 
    encoderEmptyBufferDone(OMX_HANDLETYPE, 
                           OMX_PTR, 
                           OMX_BUFFERHEADERTYPE*);
    
    static OMX_ERRORTYPE
    encoderFillBufferDone(OMX_HANDLETYPE,
                          OMX_PTR,
                          OMX_BUFFERHEADERTYPE*);
   
    
    void onEncoderPortSettingsChanged();
    void onEncoderEmptyBuffer();
    void onEncoderFillBuffer();
    int pixelSize;
    ofBuffer fileBuffer;
    string filePath;
    bool available;
    bool fileNeedsWritten;
    int startTime;
};
