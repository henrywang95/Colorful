#include "ofApp.h"
#include "ofxOilPaint.h"

//--------------------------------------------------------------
void ofApp::setup() {
    // Start the depth sensor.
    kinect.setRegistration(true);
    kinect.init();
    kinect.open();
    
    // Setup the parameters.
    nearThreshold.set("Near Threshold", 0.01f, 0.0f, 0.1f);
    farThreshold.set("Far Threshold", 0.02f, 0.0f, 0.1f);
    
    // Setup the gui.
    guiPanel.setup("Depth Threshold", "settings.json");
    guiPanel.add(nearThreshold);
    guiPanel.add(farThreshold);
    
    // Set the background color
    backgroundColor = ofColor(255);
    
    // Initialize the canvas where we are going to paint
    canvas.allocate(ofGetWidth(), ofGetHeight(), GL_RGB, 3);
    canvas.begin();
    ofClear(backgroundColor);
    canvas.end();
    
    // Initialize the application variables
    alphaValue = 0;
    nextPathLength = 0;
    
    hasBrush = false;
    //    addBrush(0, 0);
    
    amplitude = 400; // this will be the range of our oscillations (-400 to 400)
    
    sinOscillator = glm::vec2(0, -amplitude);
    cosOscillator = glm::vec2(-amplitude, 0);
    
    guiPanel.loadFromFile("settings.json");
    
}

//--------------------------------------------------------------
void ofApp::update() {
    kinect.update();
    ofFloatPixels rawDepthPix = kinect.getRawDepthPixels();
    ofFloatPixels thresholdNear, thresholdFar, thresholdResult;
    ofxCv::threshold(rawDepthPix, thresholdNear, nearThreshold);
    ofxCv::threshold(rawDepthPix, thresholdFar, farThreshold, true);
    ofxCv::bitwise_and(thresholdNear, thresholdFar, thresholdResult);
    
    // Upload pixels to image.
    thresholdImg.setFromPixels(thresholdResult);
    
    // Draw the result image.
    //thresholdImg.draw(640, 0);
    contourFinder.findContours(thresholdImg);
    
    if (contourFinder.size()){
        glm::vec2 mousePos = ofxCv::toOf(contourFinder.getCenter(0));
        
        float scaleX = ofGetWidth() / kinect.getWidth();
        float scaleY = ofGetHeight() / kinect.getHeight();
        
        mousePos *= glm::vec2(scaleX, scaleY);
        
        if (!hasBrush) {
            float depth = kinect.getDistanceAt(ofxCv::toOf(contourFinder.getCenter(0)));
            addBrush(mousePos.x, mousePos.y, depth);
            //depth = ofMap();
            //brush = ofxOilBrush(mousePos, ofRandom(30, 100));
            
            hasBrush = true;
        }
        else
        {
            dragBrush(mousePos.x,mousePos.y);
        }
        
        // Get the canvas pixels
        canvas.readToPixels(canvasPixels);
        int width = canvasPixels.getWidth();
        int height = canvasPixels.getHeight();
        
        // Get the cursor current path length
        float currentPathLength = cursorPath.getPerimeter();
        
        // Paint the brush on the canvas, starting from the last painted point
        canvas.begin();
        float time = ofGetElapsedTimef();
        
        // map oscillation frequency to mouse position
        sinFrequency = ofMap(mousePos.x, 0, ofGetWidth(), 1., 3.);
        cosFrequency = ofMap(mousePos.y, 0, ofGetHeight(), 1., 3.);
        
        // calculate oscillations
        float sinTime = sin(time * sinFrequency*0.2) * amplitude;
        float cosTime = cos(time * cosFrequency*0.2) * amplitude;
        
        
        sinOscillator   = glm::vec2(-amplitude, sinTime);    // sin oscillates on y
        cosOscillator    = glm::vec2(cosTime, -amplitude);    // cos oscillates on y
        
        mixOscillator   = glm::vec2(cosTime, sinTime);      // circular oscillation
        
        // make a trail using ofPolyline
        // https://openframeworks.cc/documentation/graphics/ofPolyline/
        
        mixTrail.addVertex(mixOscillator.x, mixOscillator.y);
        
        if (mixTrail.getVertices().size() > 400) {
            mixTrail.clear(); // reset
        }
        while (nextPathLength < currentPathLength) {
            // Update the brush position
            const glm::vec2& pathPoint = cursorPath.getPointAtLength(nextPathLength);
            brush.updatePosition(pathPoint, true);
            
            // Get the bristle positions
            const vector<glm::vec2>& bristlePositions = brush.getBristlesPositions();
            
            // Mix the current bristle colors with the color under the bristles positions
            for (unsigned int i = 0; i < bristlePositions.size(); ++i) {
                // Check that the bristle is inside the canvas
                const glm::vec2& pos = bristlePositions[i];
                int x = pos.x;
                int y = pos.y;
                
                if (x >= 0 && x < width && y >= 0 && y < height) {
                    // Get the color under the bristle
                    const ofColor& color = canvasPixels.getColor(x, y);
                    
                    // Check if we are over a pixel that has been painted already
                    if (color != backgroundColor) {
                        // Mix the current bristle color with the painted color
                        currentBristleColors[i].lerp(color, 0.001);
                        
                        // Add some of the initial color
                        currentBristleColors[i].lerp(initialBristleColors[i], 0.001);
                    } else {
                        // Add some of the initial color
                        currentBristleColors[i].lerp(initialBristleColors[i], 0.05);
                    }
                }
            }
            
            // Decrease the alpha value in each step
            alphaValue -= 1;
            
            // Paint the brush on the canvas
            if (alphaValue > 0) {
                brush.paint(currentBristleColors, alphaValue);
            }
            
            // Move to the next path length value
            nextPathLength += 12;
        }
        canvas.end();
        
    }
    else {
        hasBrush = false;
    }
}

//--------------------------------------------------------------
void ofApp::draw() {
    // Get the point distance using the SDK function.
    // float distAtMouse = kinect.getDistanceAt(ofGetMouseX(), ofGetMouseY());
    // ofDrawBitmapStringHighlight(ofToString(distAtMouse, 3), ofGetMouseX(), ofGetMouseY());
    // Threshold the depth.
    
    
    
    //  kinect.getDepthTexture().draw(0, 0);
    
    //    thresholdImg.draw(640, 0);
    //    contourFinder.draw();
    
    // Paint the canvas on the application window
//    ofBackground(255);
    canvas.draw(0, 0,ofGetWidth(), ofGetHeight());
    if (dubugMode){
        
        ofFloatPixels rawDepthPix = kinect.getRawDepthPixels();
        ofFloatPixels thresholdNear, thresholdFar, thresholdResult;
        ofxCv::threshold(rawDepthPix, thresholdNear, nearThreshold);
        ofxCv::threshold(rawDepthPix, thresholdFar, farThreshold, true);
        ofxCv::bitwise_and(thresholdNear, thresholdFar, thresholdResult);
        thresholdImg.setFromPixels(thresholdResult);
        thresholdImg.update();
        
        //kinect.getDepthTexture().draw(0, 0);
        thresholdImg.draw(0, 0);
        
        // Draw the result image.
        //  thresholdImg.draw(640, 0);
        contourFinder.findContours(thresholdImg);
        contourFinder.draw();
        guiPanel.draw();
        string message ="";
        message += "\n\n move mouse to change oscillation frequencies";
        message += "\n cos frequency (X oscillation): " + ofToString(cosFrequency);
        message += "\n sin frequency (Y oscillation): " + ofToString(sinFrequency);
        ofDrawBitmapString(message, 20, 60);
    }
    // now, move our drawing "origin" (0,0) from the top right of the window
    // to center of window temporarily:
    ofTranslate(ofGetWidth() * .5, ofGetHeight() * .5);
    
    // draw the trail:
    //ofSetColor(ofColor::white);
    ofSetColor(255,255,255,100);
    ofSetLineWidth(5);
    mixTrail.draw();
    
    ofSetColor(255);
    ofDrawCircle(mixOscillator, 5);
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if (key == ' ')
    {
        dubugMode = !dubugMode;
    }
    if (key == 'f')
    {
        ofToggleFullscreen();
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
    dragBrush(x,y);
}

void ofApp::dragBrush(int x, int y){
    // Check that we moved enough
    glm::vec2 mousePos = glm::vec2(x, y);

    if (glm::distance(mousePos, lastAddedPoint) > 2.5) {
        // Add the point to the path
        cursorPath.curveTo(mousePos.x, mousePos.y);
        //cursorPath.curveTo(mouseX, mouseY);
        
        // Save the last added point position
        lastAddedPoint = mousePos;
    }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
    // Create a new brush
    addBrush(x, y, 0);
}

void ofApp::addBrush(int x, int y, int z)
{
    glm::vec2 mousePos = glm::vec2(x, y);
    //    float brushLenth = ofMap(z, 0.013f, 0.018f, 30, 100);
    //    brush = ofxOilBrush(mousePos, brushLenth);
    brush = ofxOilBrush(mousePos, ofRandom(50, 70));
   

    // Calculate the brush bristles colors
    initialBristleColors.clear();
    float hueValue = ofRandom(255);
    
    for (unsigned int i = 0, nBristles = brush.getNBristles(); i < nBristles; ++i) {
        initialBristleColors.push_back(ofColor::fromHsb(hueValue, 200, 180 + ofRandom(-20, 20)));
    }
    
    currentBristleColors = initialBristleColors;
    
    // Set the initial alpha value
    //    float brushLenth = ofMap(z, 0.01f, 0.02f, 100, 255);
    //    alphaValue = (brushLenth);
    alphaValue = 255;
    
    // Start a new cursor path at the mouse position
    cursorPath.clear();
    //cursorPath.curveTo(mousePos.x, mousePos.y);
    lastAddedPoint = mousePos;
    
    // Reset the next path length variable
    nextPathLength = 0;
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
    // Finish the cursor path
    //  cursorPath.curveTo(ofGetMouseX(), ofGetMouseY());
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {
    
}
