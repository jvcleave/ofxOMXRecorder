#include "ofApp.h"


ofTexture pixelOutput;

int startTime = 0;
int endTime = 0;

void ofApp::setup(){
    
    ofSetLogLevel(OF_LOG_VERBOSE);
    consoleListener.setup(this);
    ofLoadImage(logo, "of.png");
    
    
    doEncode = false;
    int numColors = 0;
    
    int width = 1280;
    int height = 720;
    colorFormat = GL_RGBA; //GL_RGBA currently required
    numColors = 4;
    ofEnableAlphaBlending();

    
    fbo.allocate(width, height, colorFormat);
    fbo.begin();
        ofClear(0);
        ofBackgroundGradient(ofColor::red, ofColor::black, OF_GRADIENT_BAR);
    fbo.end();

/*
    IMAGE_TYPE
 
    Working formats:
    GIF
    PNG
    JPG
    
    Almost working:
    BMP
    
    Failing (but should work):
    TGA
    PPM
    TGA
*/
    ofxOMXImageEncoderSettings::IMAGE_TYPE imageType = ofxOMXImageEncoderSettings::JPG;
    
    ofxOMXImageEncoderSettings encoderSettings;
    encoderSettings.width       = width;        //default 1280, max 1280
    encoderSettings.height      = height;       //default 720, max 720
    encoderSettings.imageType   = imageType;
    if (imageType == ofxOMXImageEncoderSettings::JPG)
    {
        encoderSettings.JPGCompressionLevel = 50; //0-100

    }
    //encoderSettings.outputWidth  = width/2;            //default 1280, max 1280
    //encoderSettings.outputHeight = height/2;           //default 720, max 720
    
    encoder.setup(encoderSettings);
    
    imageCounter = 0;
    ofDirectory savedImagesFolder("savedImages");
    if(!savedImagesFolder.exists())
    {
        savedImagesFolder.create();
    }
    
    string folderPath = ofToDataPath(savedImagesFolder.getAbsolutePath()+ "/" +encoderSettings.getImageTypeString(), true); //e.g. bin/data/savedImages/jpg
    imagesFolder = ofDirectory(folderPath);
    if(!imagesFolder.exists())
    {
        imagesFolder.create();
    }
    
    int dataSize = width * height * numColors;
    pixels = new unsigned char[dataSize];
    memset(pixels, 0xff, dataSize); //set to white

    pixelsOF.setFromExternalPixels(pixels, width, height, numColors);
}

void ofApp::update()
{
    bool didUpdatePixels = false;
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
        if (encoder.isAvailable()) 
        {
            didUpdatePixels = true;
            glReadPixels(0,0, fbo.getWidth(), fbo.getHeight(), colorFormat, GL_UNSIGNED_BYTE, pixels);
        }
    fbo.end();
    
    if(doEncode)
    {
            
        if (encoder.isAvailable() && didUpdatePixels) 
        {
           
            encoder.encode(imagesFolder.getAbsolutePath() + "/" + ofToString(imageCounter), pixels);
            imageCounter++;
        }

        
        if(imageCounter == 10)
        {
            doEncode = false;
            endTime = ofGetElapsedTimeMillis();
            ofLogVerbose() << imageCounter << " IMAGES TOOK OMX MS: " << endTime-startTime;
            
            
            //ofSaveImage for speed comparision
            
            int startTimeOF = ofGetElapsedTimeMillis();
            ofSaveImage(pixelsOF, ofToDataPath("OF_SAVE.png", true));
            ofLogVerbose() << "OF IMAGE TOOK MS: " << ofGetElapsedTimeMillis()-startTimeOF;
        }
        
    }

}

void ofApp::draw()
{

    fbo.draw(0, 0);
    
    
    stringstream info;
    info << "App FPS: " << ofGetFrameRate() << endl;
    info << "PRESS 1 to START ENCODE " << endl;
    
    ofDrawBitmapStringHighlight(info.str(), 100, 100, ofColor::black, ofColor::yellow);
    
}

void ofApp::keyPressed(int key){

    if (key == '1') 
    {
        startTime = ofGetElapsedTimeMillis();
        doEncode = true;
    }
    if (key == '0')
    {
        imageCounter = 0;
    }
}

