#include "ofApp.h"


ofTexture pixelOutput;

void ofApp::setup(){
    
    ofSetLogLevel(OF_LOG_VERBOSE);
    consoleListener.setup(this);
    ofLoadImage(logo, "of.png");
    
    
    doStartRecording = false;
    doStopRecording = false;
    numColors = 0;
    colorFormat = GL_RGB;
    
  
    int width = 640;
    int height = 480;
    colorFormat = GL_RGB;
    
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
    
    recorder.setup(width, height, colorFormat);

    
    int dataSize = width * height * numColors;
    pixels = new unsigned char[dataSize];
    memset(pixels, 0xff, dataSize);

    pixelOutput.allocate(width, height, colorFormat);
}

void ofApp::update()
{
    fbo.begin();
        logo.draw(ofRandom(0, fbo.getWidth()), ofRandom(0, fbo.getHeight()), 20, 20);
    
        int w = fbo.getWidth()/4;
        int h = fbo.getHeight();
        int a = 255;
        ofPushStyle();
        ofPushMatrix();
    
        ofSetColor(ofColor::red);
        ofDrawRectangle(0, 0, w, h);
        
        ofTranslate(w, 0);
        ofSetColor(ofColor::blue, a*.75);
        ofDrawRectangle(0, 0, w, h);
        
        ofTranslate(w, 0);
        ofSetColor(ofColor::green, a*.50);
        ofDrawRectangle(0, 0, w, h);
        
        ofTranslate(w, 0);
        ofSetColor(ofColor::white, a*.25);
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
        recorder.startRecording();
    }
    
    if(recorder.isRecording())
    {
        recorder.update(pixels);
    }
    
    if (doStopRecording) 
    {
       doStopRecording = false;
        recorder.stopRecording();
    }
}

void ofApp::draw(){
    fbo.draw(0, 0);
    
    stringstream info;
    info << "App FPS: " << ofGetFrameRate() << endl;
    info << "IS RECORDING: " << recorder.isRecording() << endl;
    
    ofColor circleColor = ofColor::green;
    if (recorder.isRecording()) 
    {
        circleColor = ofColor::red;
    }
    ofPushStyle();
        ofSetColor(circleColor);
        ofDrawCircle(ofGetWidth() - 80, 40, 40);
    ofPopStyle();
    /*if(doStartRecording)
    {
        pixelOutput.loadData(pixels, fbo.getWidth(), fbo.getHeight(), colorFormat);
        pixelOutput.draw(fbo.getWidth(), 20, fbo.getWidth(), fbo.getHeight());
    }*/

    
    
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

