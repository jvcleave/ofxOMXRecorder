#pragma once

#include "ofMain.h"
#include "TerminalListener.h"
#include "ofxOMXImageEncoder.h"

class ofApp : public ofBaseApp, public KeyListener{
public:
    
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    
    TerminalListener consoleListener;
    void onCharacterReceived(KeyListenerEventData& e)
    {
        keyPressed((int)e.character);
    };
    
    
    ofFbo fbo;
    ofTexture logo;

    ofxOMXImageEncoder encoder;

    GLint colorFormat;
    unsigned char* pixels;
    int imageCounter;
    
    bool doEncode;
    bool doRestart;
    ofPixels pixelsOF;
    
    ofDirectory imagesFolder;
    vector<ofxOMXImageEncoderSettings::IMAGE_TYPE> imageTypes;
    int currentEncoderID;
    void setupEncoder(int id);
    
    int width;
    int height;
    int resizeWidth;
    int resizeHeight;
};


