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
    
    int width = 640;
    int height = 480;
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
    
#if 0
    ofxOMXImageEncoderSettings pngEncoderSettings;
    pngEncoderSettings.width  = width;            //default 1280, max 1280
    pngEncoderSettings.height = height;           //default 720, max 720
    pngEncoderSettings.colorFormat = colorFormat; //default GL_RGBA or GL_RGB    
    pngEncoderSettings.imageType = ofxOMXImageEncoderSettings::PNG;  
    encoder.setup(pngEncoderSettings);
#endif
    
#if 1
    ofxOMXImageEncoderSettings jpgEncoderSettings;
    jpgEncoderSettings.width  = width;            //default 1280, max 1280
    jpgEncoderSettings.height = height;           //default 720, max 720
    jpgEncoderSettings.colorFormat = colorFormat; //default GL_RGBA or GL_RGB    
    jpgEncoderSettings.imageType = ofxOMXImageEncoderSettings::JPG; 
    jpgEncoderSettings.JPGCompressionLevel = 50;
    encoder.setup(jpgEncoderSettings);
#endif
    
    imageCounter = 0;
   
    
    
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
        string absoluteFilePath;
        string folderPath = ofToDataPath("savedImages", true);
        
        ofDirectory imageFolder(folderPath);
        
        if(!imageFolder.exists())
        {
            imageFolder.create();
        }
        
        if (encoder.isAvailable() && didUpdatePixels) 
        {
           
            absoluteFilePath= imageFolder.getAbsolutePath() + "/" + ofToString(imageCounter);
            encoder.encode(absoluteFilePath, pixels);
            imageCounter++;
        }

        
        if(imageCounter == 10)
        {
            doEncode = false;
            endTime = ofGetElapsedTimeMillis();
            ofLogVerbose() << imageCounter << " IMAGES TOOK MS: " << endTime-startTime;
            
            int startTimeOF = ofGetElapsedTimeMillis();
            stringstream filePath;
            filePath << "savedImages/";
            filePath << "OF_SAVE";
            filePath << ".png";
            ofSaveImage(pixelsOF, ofToDataPath(filePath.str(), true));
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

