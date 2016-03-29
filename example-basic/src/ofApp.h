#pragma once

#include "ofMain.h"
#include "TerminalListener.h"
#include "ofxOMXRecorder.h"

class ofApp : public ofBaseApp, public KeyListener{
public:
    
    ofFbo fbo;
    ofTexture logo;
    ofxOMXRecorder recorder;
    unsigned char* pixels;
    int numColors;
    
    bool doStartRecording;
    bool doStopRecording;

    bool hasWrittenFile;
    GLint colorFormat;

    
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    
    TerminalListener consoleListener;
    void onCharacterReceived(KeyListenerEventData& e)
    {
        keyPressed((int)e.character);
    };
};
