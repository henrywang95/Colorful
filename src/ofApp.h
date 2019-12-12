#pragma once

#include "ofMain.h"
#include "ofxOilPaint.h"

#include "ofxCv.h"
#include "ofxKinect.h"

#include "ofxOsc.h"
#include "ofxAbletonLive.h"

#include "ofxGui.h"

class ofApp: public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    
    ofxOscSender sender;
    ofxOscReceiver receiver;
    
    ofSoundPlayer   mySound;
    
    ofxAbletonLive live;
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    void addBrush(int x, int y, int z);
    void dragBrush(int x, int y);
    void stopBrush();
    
    ofxKinect kinect;
    
    ofImage thresholdImg;
    ofxCv::ContourFinder contourFinder;
    
    ofParameter<float> nearThreshold;
    ofParameter<float> farThreshold;
    
    ofxPanel guiPanel;
    bool hasBrush;
    bool dubugMode;
    ofColor backgroundColor;
    ofFbo canvas;
    ofPixels canvasPixels;
    ofxOilBrush brush;
    vector<ofColor> initialBristleColors;
    vector<ofColor> currentBristleColors;
    float alphaValue;
    ofPolyline cursorPath;
    glm::vec2 lastAddedPoint;
    float nextPathLength;
    
    // locations to Oscillatorillate
    
    glm::vec2 sinOscillator;
    glm::vec2 cosOscillator;
    glm::vec2 mixOscillator; // we'll mix sin and cos Oscillatorillation over x and y
    
    float amplitude, sinFrequency, cosFrequency;
    
    ofPolyline mixTrail;    // we'll store the mixOscillator positions
    // over time to draw a trail
};
