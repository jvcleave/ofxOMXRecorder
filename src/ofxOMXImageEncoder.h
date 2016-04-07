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
    
    int outputWidth;
    int outputHeight;
    ofxOMXImageEncoderSettings()
    {
        width = 1280;
        height = 720;
        colorFormat = GL_RGBA;
        bitrateMegabytesPerSecond = 2.0;
        enablePrettyFileName = true;
        imageType = PNG;
        JPGCompressionLevel = 30;
        outputWidth = width;
        outputHeight = height;
        
    };
    void validate()
    {
        int currentOutputWidth = outputWidth;
        int currentOutputHeight = outputHeight;
        if (currentOutputWidth > width)
        {
            outputWidth = width;
        }
        
        if (currentOutputHeight > height)
        {
            outputHeight = height;
        }
    }
    string getImageTypeString()
    {
        string typeString = "UNSET";
        switch (imageType) 
        {
            case BMP: typeString = "bmp"; break;
            case GIF: typeString = "gif"; break;
            case TGA: typeString = "tga"; break;
            case PPM: typeString = "ppm"; break;
            case JPG: typeString = "jpg"; break;
            case PNG: typeString = "png"; break;
        }
        return typeString;
    }
    string getFileExtension()
    {
        return "." + getImageTypeString();
    }
    string toString()
    {
        stringstream info;
        info << endl;
        info << "width: " << width << endl;
        info << "height: " << height << endl;
        info << "outputWidth: " << outputWidth << endl;
        info << "outputHeight: " << outputHeight << endl;
        info << "bitrateMegabytesPerSecond: " << bitrateMegabytesPerSecond << endl;
        info << "imageType: " << getImageTypeString() << endl;
        info << "enablePrettyFileName: " << enablePrettyFileName << endl;
        info << "JPGCompressionLevel: " << JPGCompressionLevel << endl;

        if (colorFormat == GL_RGBA)
        {
            info << "colorFormat: " << "GL_RGBA" << endl;

        }
        if (colorFormat == GL_RGB)
        {
            info << "colorFormat: " << "GL_RGB" << endl;
            
        }
        return info.str();
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
    
    ofxOMXImageEncoderSettings& getSettings()
    {
        return settings;
    }
private:
    void checkPorts(bool doBuffers=true);
    void resetValues();
    void teardown();
    ofxOMXImageEncoderSettings settings;
    OMX_HANDLETYPE encoder;
    OMX_BUFFERHEADERTYPE* resizeInputBuffer;
    OMX_BUFFERHEADERTYPE* resizeOutputBuffer;

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
    
    //resizer
    OMX_HANDLETYPE resizer;
    static OMX_ERRORTYPE 
    resizerEmptyBufferDone(OMX_HANDLETYPE, 
                           OMX_PTR, 
                           OMX_BUFFERHEADERTYPE*);
    
    static OMX_ERRORTYPE
    resizerFillBufferDone(OMX_HANDLETYPE,
                          OMX_PTR,
                          OMX_BUFFERHEADERTYPE*);
    
    static OMX_ERRORTYPE 
    resizerEventHandlerCallback(OMX_HANDLETYPE, 
                                OMX_PTR, 
                                OMX_EVENTTYPE, 
                                OMX_U32, OMX_U32, 
                                OMX_PTR);
    void onResizerEmptyBuffer();
    void onResizerFillBuffer();
    void onResizerPortSettingsChanged();
    
    void onEncoderPortSettingsChanged();
    void onEncoderEmptyBuffer();
    void onEncoderFillBuffer();
    int pixelSize;
    ofBuffer fileBuffer;
    string filePath;
    bool available;
    bool fileNeedsWritten;
    int startTime;
    
    void probeEncoder();
};
