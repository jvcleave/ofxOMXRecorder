#include "ofApp.h"


ofTexture pixelOutput;

void ofApp::setup(){
    
    ofSetLogLevel(OF_LOG_VERBOSE);
    consoleListener.setup(this);
    ofLoadImage(logo, "of.png");
    
    
    doStartRecording = false;
    doStopRecording = false;
    int numColors = 0;
    
    int width = 1280;
    int height = 720;
    colorFormat = GL_RGBA;
    

    
    if (colorFormat == GL_RGB) 
    {
        numColors = 3;
    }
    if (colorFormat == GL_RGBA) 
    {
        numColors = 4;
        ofEnableAlphaBlending();
    }
    fbo.allocate(width, height, colorFormat);
    fbo.begin();
        ofClear(0);
        ofBackgroundGradient(ofColor::red, ofColor::black, OF_GRADIENT_BAR);
    fbo.end();
    
    
    ofxOMXRecorderSettings settings;
    settings.width  = width;            //default 1280, max 1280
    settings.height = height;           //default 720, max 720
    settings.fps    = 30;               //default 30, max 30
    settings.colorFormat = colorFormat; //default GL_RGBA or GL_RGB 
    settings.bitrateMegabytesPerSecond   = 2.0;  //default 2.0, max untested
    settings.enablePrettyFileName           = true; //default true
    recorder.setup(settings);

    
    int dataSize = width * height * numColors;
    pixels = new unsigned char[dataSize];
    memset(pixels, 0xff, dataSize); //set to white

}

void ofApp::update()
{
    fbo.begin();
        logo.draw(ofRandom(0, fbo.getWidth()), ofRandom(0, fbo.getHeight()), 20, 20);
    
        int w = fbo.getWidth()/8;
        int h = fbo.getHeight()/2;
        int a = 255;
        ofPushStyle();
        ofPushMatrix();
    
        ofTranslate(w, 0);
        ofSetColor(ofColor::red);
        ofDrawRectangle(0, 0, w, h);
        
        ofTranslate(w, 0);
        ofSetColor(ofColor::blue, a * 0.75f);
        ofDrawRectangle(0, 0, w, h);
        
        ofTranslate(w, 0);
        ofSetColor(ofColor::green, a * 0.5f);
        ofDrawRectangle(0, 0, w, h);
        
        ofTranslate(w, 0);
        ofSetColor(ofColor::white, a * 0.25f);
        ofDrawRectangle(0, 0, w, h);
    
        ofPopMatrix();
        ofPopStyle();
        if(recorder.isRecording())
        {
            glReadPixels(0,0, fbo.getWidth(), fbo.getHeight(), colorFormat, GL_UNSIGNED_BYTE, pixels);
        }
    fbo.end();
    
    if(doStartRecording)
    {
        doStartRecording = false;
        
        
        /*
         Recording filename options:
         Pretty mode: width, height, fps, bitrate, number of frames in name:
         Send empty and pretty version is generated (unless settings.enablePrettyFileName == false)
         */
        
        recorder.startRecording();
        
        
        //pass in your own
        /*
         string absoluteFilePath;
         absoluteFilePath= ofToDataPath(ofGetTimestampString()+".h264", true);
         recorder.startRecording(absoluteFilePath);
        */
        
    }
    
    if (doStopRecording) 
    {
        doStopRecording = false;
        recorder.stopRecording();
    }
    
    /*
     IMPORTANT
     
     Even though you may have selected recorder.stopRecording()
     The file needs to end on a keyframe in order to be valid
     
     Continue sending pixels until recorder.isRecording() == false
     
     */
     
    if(recorder.isRecording())
    {
        recorder.update(pixels);
    }

}

void ofApp::draw()
{

    fbo.draw(0, 0);
    
    bool isRecording = recorder.isRecording();
    
    stringstream info;
    info << "App FPS: " << ofGetFrameRate() << endl;
    info << "PRESS 1 to START RECORDING "   << isRecording << endl;
    info << "PRESS 2 to STOP RECORDING "    << isRecording << endl;
    if (isRecording) 
    {
        info << "FRAMES RECORDED: "    << recorder.getFrameCounter() << endl;

    }
    if (!recorder.recordings.empty()) 
    {
        info << "RECORDED FILES: " << endl;
        for(size_t i=0; i<recorder.recordings.size(); i++)
        {
            info << i << ":" <<  recorder.recordings[i].path() << endl;
        }
    }
    
    ofColor circleColor = ofColor::green;
    if (isRecording) 
    {
        circleColor = ofColor::red;
    }
    ofPushStyle();
        ofSetColor(circleColor);
        ofDrawCircle(ofGetWidth() - 80, 40, 40);
    ofPopStyle();
    
    ofDrawBitmapStringHighlight(info.str(), 100, 100, ofColor::black, ofColor::yellow);
    
}

void ofApp::keyPressed(int key){

    if (key == '1') 
    {
        doStartRecording = true;
    }
    if (key == '2') 
    {
        doStopRecording = true;
    }
}

