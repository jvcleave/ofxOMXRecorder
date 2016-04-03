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
    ofPixels pixelsOF;
    
    ofDirectory imagesFolder;
};


