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
    
    //Pixel constructors
    Pixel(int xCoord, int yCoord, int r, int g, int b): x(xCoord), y(yCoord), red(r), green(g), blue(b) {}
    Pixel(): x(0), y(0) {}
};

Pixel mapPointToPixel (Point point, double scaleFactor) {
    struct Pixel newPixel;
    newPixel.x = point.x * scaleFactor + g_width / 2;
    newPixel.y = point.y * scaleFactor + g_height / 2;
    
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
//    if(argc < 3) {
//      cout << "Usage: Assignment1 meshfile imagefile" << endl;
//      return 0;
//   }
	// OBJ filename - uncomment this to submit.
//    string meshName(argv[1]);
//    string imgName(argv[2]);
    
    // Right now, hard coding all inputs:
    string imgName("out.PNG");
    g_width = 200; //set g_width and g_height appropriately!
    g_height = 200;
    
    Pixel vertexA = Pixel(0, 0, 255, 0, 0);
    Pixel vertexB = Pixel(50, 50, 0, 255, 0);
    Pixel vertexC = Pixel(100, 0, 0, 0, 255);
    
    Pixel triangleVertices [3];
    
    triangleVertices[0] = vertexA;
    triangleVertices[1] = vertexB;
    triangleVertices[2] = vertexC;
    

   //create an image - an unsigned character array with 3
	auto image = make_shared<Image>(g_width, g_height);

    // Print out the 3 points
//    for (int i = 0; i < 3; i++){
//        image -> setPixel(triangleVertices[i].x, triangleVertices[i].y, triangleVertices[i].red, triangleVertices[i].green, triangleVertices[i].blue);
//    }
    
    
    // First, find the bounding box of all the obj coordinates in coordinate space
    int maxX = -1;
    int maxY = -1;
    int minX = -1;
    int minY = -1;

    // Iterate through the posBuf array of coordinates and pick the max and min for X and Y
    for(int i = 0; i< 3; i++){
        if(maxX == -1|| maxX < triangleVertices[i].x) {
            maxX = triangleVertices[i].x;
        }
        // If for minX
        cout << "For current X:" << triangleVertices[i].x << endl;
        cout << "Where minX is set to:" << minX << endl;
        if(minX == -1|| minX > triangleVertices[i].x) {
            minX = triangleVertices[i].x;
            cout << "Set to the new minx:" << minX << endl;
        }
        // If for maxY
        if(maxY == -1 || maxY < triangleVertices[i].y) {
            maxY = triangleVertices[i].y;
        }
        if(minY == -1 || minY > triangleVertices[i].y) {
            minY = triangleVertices[i].y;
        }
    }
    
    // Print them out
    cout << "New maxX set to:" << maxX << endl;
    cout << "New minX set to:" << minX << endl;
    cout << "New maxY set to:" << maxY << endl;
    cout << "New minY set to:" << minY << endl;


    // Create lowerBoundingBox and upperBoundingBox

    // Create a lower bounding pixel
    struct Pixel lowerBoundingPixel;
    lowerBoundingPixel.x = minX;
    lowerBoundingPixel.y = minY;
    cout << "The lowerBoundingPixel:" << " (" << lowerBoundingPixel.x << "," << lowerBoundingPixel.y << ")" << endl;

    // Create an upper bounding pixel
    struct Point upperBoundingPixel;
    upperBoundingPixel.x = maxX;
    upperBoundingPixel.y = maxY;
    cout << "The upperBoundingPixel:" << " (" << upperBoundingPixel.x << "," << upperBoundingPixel.y << ")" << endl;

    // What colors should we give it?
    Pixel a = triangleVertices[0];
    Pixel b = triangleVertices[1];
    Pixel c = triangleVertices[2];

    double totalArea = areaOfTriangle(a, b, c);
    cout << "The TOTAL AREA:" << totalArea << endl;

    // Loop through and print each inside bounding box to draw a rectangle
    for(int x = lowerBoundingPixel.x; x < upperBoundingPixel.x; x++){
        for(int y = lowerBoundingPixel.y; y < upperBoundingPixel.y; y++) {

            struct Pixel currentPix;
            currentPix.x = x;
            currentPix.y = y;

            if (PointInTriangle(currentPix, a, b, c)){
                
                double baryA = areaOfTriangle(b, c, currentPix) / totalArea;
                double baryB = areaOfTriangle(a, c, currentPix) / totalArea;
                double baryC = areaOfTriangle(a, b, currentPix) / totalArea;
                
                cout << "baryA: " << baryA << endl;
                cout << "baryB: " << baryB << endl;
                cout << "baryC: " << baryC << endl;
                
                currentPix.red = (baryA * a.red) + (baryB * b.red) + (baryC * c.red);
                currentPix.green = (baryA * a.green) + (baryB * b.green) + (baryC * c.green);
                currentPix.blue = (baryA * a.blue) + (baryB * b.blue) + (baryC * c.blue);
                
                image -> setPixel(currentPix.x, currentPix.y, currentPix.red, currentPix.green, currentPix.blue);
                
            }
        }
    }
    
    // NOTES: vector is a dynamic array of unsigned characters
    
	// triangle buffer - an array list
	vector<unsigned int> triBuf;
    
	// position buffer
    vector<float> posBuf;   //NOTES: This is our vertices!
    
	// Some obj files contain material information.
	// We'll ignore them for this assignment.
	vector<tinyobj::shape_t> shapes; // geometry
	vector<tinyobj::material_t> objMaterials; // material
	string errStr;
    
	//write out the image
   image->writeToFile(imgName);

	return 0;
}
