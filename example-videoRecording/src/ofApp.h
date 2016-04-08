#pragma once

#include "ofMain.h"
#include "TerminalListener.h"
#include "ofxOMXRecorder.h"

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
    ofxOMXRecorder recorder;
    GLint colorFormat;
    unsigned char* pixels;
    
    
    bool doStartRecording;
    bool doStopRecording;
};


