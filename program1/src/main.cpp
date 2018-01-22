/* Release code for program 1 CPE 471 Fall 2016 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cmath>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "Image.h"

#define MIN(a,b)    ((a) < (b) ? (a) : (b))
#define MAX(a,b)    ((a) > (b) ? (a) : (b))

// This allows you to skip the `std::` in front of C++ standard library
// functions. You can also say `using std::cout` to be more selective.
// You should never do this in a header file.
using namespace std;

int g_width, g_height;

/*
   Helper function you will want all quarter
   Given a vector of shapes which has already been read from an obj file
   resize all vertices to the range [-1, 1]
 */

struct Point {
    double x;
    double y;
    double z;
};

struct Pixel {
    int x;
    int y;
    int red;
    int green;
    int blue;
    double alpha;
    
    //Pixel constructors
    Pixel(int xCoord, int yCoord, int r, int g, int b): x(xCoord), y(yCoord), red(r), green(g), blue(b), alpha(1.0) {}
    Pixel(int xCoord, int yCoord): x(xCoord), y(yCoord), alpha(1.0){}
    Pixel(): x(0), y(0), alpha(1.0) {}
    
    void setGradient(Pixel topColor, Pixel bottomColor ) {
        red = topColor.red;
        green = topColor.green;
        blue = topColor.blue;
    }
};

Pixel mapPointToPixel (Point point, double scaleFactor, double zDepth, double minZ, double objectHeight, double minY) {
    struct Pixel newPixel;
    newPixel.x = point.x * scaleFactor + g_width / 2;
    newPixel.y = point.y * scaleFactor + g_height / 2;
    
    // FOR Z-Buffer implementation:
    // Calulcate alpha value
    newPixel.alpha = (point.z - minZ) / zDepth;
//    cout << "For (" << newPixel.x<< "," << newPixel.y<< "): Alpha: "<< newPixel.alpha << ", zDepth: "<< zDepth<< ", point.z: "<< point.z<< ", minZ: "<< minZ <<endl;
    
    // Calulate color valus based on alpha value
    newPixel.red = 255 * newPixel.alpha;
    
    
    // For COLOR MODE implementation
//    newPixel.red =
//
    
    return newPixel;
};

int sign (Pixel p1, Pixel p2, Pixel p3) {
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool PointInTriangle (Pixel pt, Pixel v1, Pixel v2, Pixel v3) {
    bool b1, b2, b3;
    
    b1 = sign(pt, v1, v2) < 0;
    b2 = sign(pt, v2, v3) < 0;
    b3 = sign(pt, v3, v1) < 0;
    
    return ((b1 == b2) && (b2 == b3));
}

double areaOfTriangle(Pixel a, Pixel b, Pixel c){
    double area = std::abs ((a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y)) / 2);
    return area;
}

int smallest(int x, int y, int z){
    return std::min({x, y, z});
}

int largest(int x, int y, int z){
    return std::max({x, y, z});
}

void resize_obj(std::vector<tinyobj::shape_t> &shapes){
   float minX, minY, minZ;
   float maxX, maxY, maxZ;
   float scaleX, scaleY, scaleZ;
   float shiftX, shiftY, shiftZ;
   float epsilon = 0.001;

   minX = minY = minZ = 1.1754E+38F;
   maxX = maxY = maxZ = -1.1754E+38F;

   //Go through all vertices to determine min and max of each dimension
   for (size_t i = 0; i < shapes.size(); i++) {
      for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
         if(shapes[i].mesh.positions[3*v+0] < minX) minX = shapes[i].mesh.positions[3*v+0];
         if(shapes[i].mesh.positions[3*v+0] > maxX) maxX = shapes[i].mesh.positions[3*v+0];

         if(shapes[i].mesh.positions[3*v+1] < minY) minY = shapes[i].mesh.positions[3*v+1];
         if(shapes[i].mesh.positions[3*v+1] > maxY) maxY = shapes[i].mesh.positions[3*v+1];

         if(shapes[i].mesh.positions[3*v+2] < minZ) minZ = shapes[i].mesh.positions[3*v+2];
         if(shapes[i].mesh.positions[3*v+2] > maxZ) maxZ = shapes[i].mesh.positions[3*v+2];
      }
   }

	//From min and max compute necessary scale and shift for each dimension
   float maxExtent, xExtent, yExtent, zExtent;
   xExtent = maxX-minX;
   yExtent = maxY-minY;
   zExtent = maxZ-minZ;
   if (xExtent >= yExtent && xExtent >= zExtent) {
      maxExtent = xExtent;
   }
   if (yExtent >= xExtent && yExtent >= zExtent) {
      maxExtent = yExtent;
   }
   if (zExtent >= xExtent && zExtent >= yExtent) {
      maxExtent = zExtent;
   }
   scaleX = 2.0 /maxExtent;
   shiftX = minX + (xExtent/ 2.0);
   scaleY = 2.0 / maxExtent;
   shiftY = minY + (yExtent / 2.0);
   scaleZ = 2.0/ maxExtent;
   shiftZ = minZ + (zExtent)/2.0;

   //Go through all verticies shift and scale them
   for (size_t i = 0; i < shapes.size(); i++) {
      for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
         shapes[i].mesh.positions[3*v+0] = (shapes[i].mesh.positions[3*v+0] - shiftX) * scaleX;
         assert(shapes[i].mesh.positions[3*v+0] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3*v+0] <= 1.0 + epsilon);
         shapes[i].mesh.positions[3*v+1] = (shapes[i].mesh.positions[3*v+1] - shiftY) * scaleY;
         assert(shapes[i].mesh.positions[3*v+1] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3*v+1] <= 1.0 + epsilon);
         shapes[i].mesh.positions[3*v+2] = (shapes[i].mesh.positions[3*v+2] - shiftZ) * scaleZ;
         assert(shapes[i].mesh.positions[3*v+2] >= -1.0 - epsilon);
         assert(shapes[i].mesh.positions[3*v+2] <= 1.0 + epsilon);
      }
   }
}

// argc is the argument list - we'll want it to be the restorizer,
int main(int argc, char **argv)
{
    if(argc < 3) {
      cout << "Usage: Assignment1 meshfile imagefile" << endl;
      return 0;
   }
	// OBJ filename - uncomment this to submit.
    string meshName(argv[1]);
    string imgName(argv[2]);
    
    // Right now, hard coding all inputs:
//    string imgName("out.PNG");
    g_width = 600; //set g_width and g_height appropriately!
    g_height = 600;
    
    Pixel topColor(0, 0, 200, 0, 50);
    Pixel bottomColor(0, 0, 0, 250, 0);
    

   //create an image - an unsigned character array with 3
	auto image = make_shared<Image>(g_width, g_height);
    
    // Create a z-buffer that keeps track of z-depths of pixels written to the screen
    double zBuffer [g_width][g_height];

    // triangle buffer
    vector<unsigned int> triBuf;
    // position buffer
    vector<float> posBuf;
    
    // Some obj files contain material information.We'll ignore them for this assignment.
    vector<tinyobj::shape_t> shapes; // geometry
    vector<tinyobj::material_t> objMaterials; // material
    string errStr;
    
    bool rc = tinyobj::LoadObj(shapes, objMaterials, errStr, meshName.c_str());
    /* error checking on read */
    if(!rc) {
        cerr << errStr << endl;
    } else {
        //keep this code to resize your object to be within -1 -> 1
        resize_obj(shapes);
        posBuf = shapes[0].mesh.positions;
        triBuf = shapes[0].mesh.indices;
    }
    cout << "Number of vertices: " << posBuf.size()/3 << endl;
    cout << "Number of triangles: " << triBuf.size()/3 << endl;
    
    // Prints out all vertices and triangles:
//    cout << "Vertices in posBuf: (" << posBuf.size()/3 << "): " << endl;
//    for(int i = 0; i< posBuf.size()/3; i++){
//        cout << i*3 << " : (" << posBuf[3* i] << ", " << posBuf[3* i + 1] << ", " << posBuf[3* i + 2] << ") "<< endl;
//    }
//
//
//    cout << "Triangles in triBuf (" << triBuf.size()/3 << "): " << endl;
//    for(int i = 0; i< triBuf.size()/3; i++){
//        cout << i*3 << " : (" << triBuf[3* i] << ", " << triBuf[3* i + 1] << ", " << triBuf[3* i + 2] << ") "<< endl;
//    }

    
    
    // First, find the bounding box of all the obj coordinates in coordinate space
    double maxX;
    double maxY = NULL;
    double maxZ = NULL;

    double minX = NULL;
    double minY = NULL;
    double minZ = NULL;
    
    
    // Iterate through the posBuf array of coordinates and pick the max and min for X and Y
    for(int i = 0; i< posBuf.size()/3; i++){
        if(!maxX || maxX < posBuf[3*i]) {
            maxX = posBuf[3*i];
        }
        if(!minX || minX > posBuf[3*i]) {
            minX = posBuf[3*i];
        }
        if(!maxY || maxY < posBuf[3*i + 1]) {
            maxY = posBuf[3*i + 1];
        }
        if(!minY || minY > posBuf[3*i + 1]) {
            minY = posBuf[3*i + 1];
        }
        if(!maxZ || maxZ < posBuf[3*i + 2]) {
            maxZ = posBuf[3*i + 2];
        }
        if(!minZ || minZ > posBuf[3*i + 2]) {
            minZ = posBuf[3*i + 2];
        }
    }
    // Print them out
    cout << "New maxX set to:" << maxX << endl;
    cout << "New minX set to:" << minX << endl;
    cout << "New maxY set to:" << maxY << endl;
    cout << "New minY set to:" << minY << endl;
    cout << "New maxZ set to:" << maxZ << endl;
    cout << "New minZ set to:" << minZ << endl;
    
    // Calculate a scale factor between the coordiate space and the 2D image
    double objLength = abs(maxX) + abs(minX); // Get the total X distance of the object
    double objHeight = abs(maxY) + abs(minY); // Get the total Y distance of the object
    double objDepth = abs(maxZ) + abs(minZ); // Get the total Z depth of the object
    
    double xScaleFactor = g_width / objLength;
    double yScaleFactor = g_height / objHeight;
    
    double scaleFactor = MIN(xScaleFactor, yScaleFactor);
    
//    // Create a lower bounding pixel
//    struct Point lowerBoundingPoint;
//    lowerBoundingPoint.x = minX;
//    lowerBoundingPoint.y = minY;
//
//    Pixel lowerBoundingPixel = mapPointToPixel(lowerBoundingPoint, scaleFactor);
//    cout << "The lowerBoundingPixel:" << " (" << lowerBoundingPixel.x << "," << lowerBoundingPixel.y << ")" << endl;
//    image -> setPixel(lowerBoundingPixel.x, lowerBoundingPixel.y, 0, 255, 0);
//
//    // Create an upper bounding pixel
//    struct Point upperBoundingPoint;
//    upperBoundingPoint.x = maxX;
//    upperBoundingPoint.y = maxY;
//
//    Pixel upperBoundingPixel = mapPointToPixel(upperBoundingPoint, scaleFactor);
//    cout << "The upperBoundingPixel:" << " (" << upperBoundingPixel.x << "," << upperBoundingPixel.y << ")" << endl;
//    image -> setPixel(upperBoundingPixel.x, upperBoundingPixel.y, 0, 255, 0);
//
//
    
    
    // Loop through all triangles
    for(int i = 0; i< triBuf.size() / 3; i++){
        
        int pointA = triBuf[3* i];
        int pointB = triBuf[3* i + 1];
        int pointC = triBuf[3* i + 2];
        
        cout << "pointA: " << pointA << " (" << posBuf[3* pointA] << "," << posBuf[3* pointA + 1] << "," << posBuf[3* pointA + 2] << ")" << endl;
        cout << "pointB: " << pointB << endl;
        cout << "pointC: " << pointC << endl;
        
        
        // Convert each point of the triangle into a 2D pixel
        
        struct Point aPoint;
        aPoint.x = posBuf[3* pointA];
        aPoint.y = posBuf[3* pointA + 1];
        aPoint.z = posBuf[3* pointA + 2];
        Pixel a = mapPointToPixel(aPoint, scaleFactor, objDepth, minZ, objHeight, minY);
//        a.setGradient(topColor, bottomColor);
        a.red = 130;
        a.blue = 255;
        a.green = 30;
        
//        cout << "Created a NEW PIXEL:" << " (" << a.x << "," << a.y << ")" << endl;
        //        image -> setPixel(a.x, a.y, 255, 0, 0);
        
        struct Point bPoint;
        bPoint.x = posBuf[3* pointB];
        bPoint.y = posBuf[3* pointB + 1];
        bPoint.z = posBuf[3* pointB + 2];
        Pixel b = mapPointToPixel(bPoint, scaleFactor, objDepth, minZ, objHeight, minY);
//        b.setGradient(topColor, bottomColor);
//        cout << "Created a NEW PIXEL:" << " (" << b.x << "," << b.y << ")" << endl;
        //        image -> setPixel(b.x, b.y, 255, 0, 0);
        b.red = 130;
        b.blue = 255;
        b.green = 30;
        
        struct Point cPoint;
        cPoint.x = posBuf[3* pointC];
        cPoint.y = posBuf[3* pointC + 1];
        cPoint.z = posBuf[3* pointC + 2];
        Pixel c = mapPointToPixel(cPoint, scaleFactor, objDepth, minZ, objHeight, minY);
//        c.setGradient(topColor, bottomColor);
        c.red = 130;
        c.blue = 255;
        c.green = 30;
//        cout << "Created a NEW PIXEL:" << " (" << c.x << "," << c.y << ")" << endl;
        //        image -> setPixel(cPixel.x, cPixel.y, 255, 0, 0);
        
        int triMinX = smallest(a.x, b.x, c.x);
        int triMaxX = largest(a.x, b.x, c.x);
        int triMinY = smallest(a.y, b.y, c.y);
        int triMaxY = largest(a.y, b.y, c.y);
        
        double totalArea = areaOfTriangle(a, b, c);
        cout << "The TOTAL AREA:" << totalArea << endl;
        
        // Now draw the triangle by iterating through each pixel inside the bounding box and checking if it lies inside the triangle formed by aPoint, bPoint, and cPoint
        for(int x = triMinX; x < triMaxX; x++){
            for(int y = triMinY; y < triMaxY; y++) {
                
                struct Pixel currentPix;
                currentPix.x = x;
                currentPix.y = y;
                
                
                // Check to see if the point is inside the triangle.
                if (PointInTriangle(currentPix, a, b, c)){
                    
                    double baryA = areaOfTriangle(b, c, currentPix) / totalArea;
                    double baryB = areaOfTriangle(a, c, currentPix) / totalArea;
                    double baryC = areaOfTriangle(a, b, currentPix) / totalArea;
                    
//                    cout << "baryA: " << baryA << endl;
//                    cout << "baryB: " << baryB << endl;
//                    cout << "baryC: " << baryC << endl;
//
                    currentPix.red = (baryA * a.red) + (baryB * b.red) + (baryC * c.red);
                    currentPix.green = (baryA * a.green) + (baryB * b.green) + (baryC * c.green);
                    currentPix.blue = (baryA * a.blue) + (baryB * b.blue) + (baryC * c.blue);
                    currentPix.alpha =(baryA * a.alpha) + (baryB * b.alpha) + (baryC * c.alpha);
                    
                    // Before drawing the new pixel, determine if it's in the foreground or background - if there is no color previously drawn, draw it. If the existing color value is less than the new, draw it. If the existing color value is MORE than the new, don't draw it.
                    // Use the image -> getPixel()  - info found on Google doc - which will allow you to get info on the existing pixel
                    
                    image -> setPixel(currentPix.x, currentPix.y, currentPix.red, currentPix.green, currentPix.blue);
//                    image -> setPixel(currentPix.x, currentPix.y, currentPix.red, 0, 0);
                    
//                    if(zBuffer[currentPix.x][currentPix.y] < currentPix.alpha){
//                        image -> setPixel(currentPix.x, currentPix.y, currentPix.red, 0, 0);
//                        zBuffer[currentPix.x][currentPix.y] = currentPix.alpha;
//                    }
                    
                    
                }
                
            }
        }
    }
    
    
	//write out the image
   image->writeToFile(imgName);

	return 0;
}
