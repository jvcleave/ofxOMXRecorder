#include "ofApp.h"


ofTexture pixelOutput;

int startTime = 0;
int endTime = 0;

void ofApp::setup()
{
    
    ofSetLogLevel(OF_LOG_VERBOSE);
    consoleListener.setup(this);
    ofLoadImage(logo, "of.png");
    
    
    doEncode = false;
    doRestart=false;
    int numColors = 0;
    
    width = 1280;
    height = 720;
    
    resizeWidth = width;
    resizeHeight = height;

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
    imageTypes.push_back(ofxOMXImageEncoderSettings::JPG);
    imageTypes.push_back(ofxOMXImageEncoderSettings::PNG);
    imageTypes.push_back(ofxOMXImageEncoderSettings::GIF);
    
    
    
    
    
    setupEncoder(0);
    
    
    int dataSize = width * height * numColors;
    pixels = new unsigned char[dataSize];
    memset(pixels, 0xff, dataSize); //set to white
    
    pixelsOF.setFromExternalPixels(pixels, width, height, numColors);
}

void ofApp::setupEncoder(int id)
{
    
    imageCounter = 0;
    
    currentEncoderID = id;
    ofxOMXImageEncoderSettings::IMAGE_TYPE imageType = imageTypes[currentEncoderID];
    
    
    
    ofxOMXImageEncoderSettings encoderSettings;
    encoderSettings.width       = width;        //default 1280, max 1280
    encoderSettings.height      = height;       //default 720, max 720
    encoderSettings.colorFormat = colorFormat;  //default GL_RGBA or GL_RGB    
    encoderSettings.imageType   = imageType;
    if (imageType == ofxOMXImageEncoderSettings::JPG)
    {
        encoderSettings.JPGCompressionLevel = 50; //0-100
        
    }
    
    encoderSettings.outputWidth  = resizeWidth;            
    encoderSettings.outputHeight = resizeHeight; 
    
    
    
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
    
    
    encoder.setup(encoderSettings);
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
            doRestart = true;
        }
    }
    if (doRestart)
    {
        if (encoder.isAvailable()) 
        {
            doRestart = false;
            endTime = ofGetElapsedTimeMillis();
            ofLogVerbose() << imageCounter << " " << encoder.getSettings().getImageTypeString() << "s TOOK OPENMAX " << endTime-startTime  <<" MS ";
            encoder.close();
            if (currentEncoderID+1 < imageTypes.size())
            {
                currentEncoderID++;
                
                setupEncoder(currentEncoderID);
                startTime = ofGetElapsedTimeMillis();
                doEncode = true;
            }else
            {
                currentEncoderID = 0;
                //ofSaveImage for speed comparision
                int startTimeOF = ofGetElapsedTimeMillis();
                string imageFilePath = ofToDataPath("OF_SAVE.png", true);
                ofSaveImage(pixelsOF, imageFilePath);
                ofLogVerbose() << imageFilePath << " TOOK ofSaveImage " << ofGetElapsedTimeMillis()-startTimeOF << " MS";
            }
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

