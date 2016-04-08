#include "ofMain.h"
#include "ofApp.h"

//programmable much slower
//#define USE_PROGRAMMABLE 
int main()
{
    int w = 1280;
    int h = 720;
    ofSetLogLevel(OF_LOG_VERBOSE);
#ifdef USE_PROGRAMMABLE
    ofGLESWindowSettings settings;
    settings.width = w;
    settings.height = h;
    settings.setGLESVersion(2);
    ofCreateWindow(settings);
#else
    ofSetupOpenGL(w, h, OF_WINDOW);
#endif

    ofRunApp( new ofApp());
}