/* Release code for program 1 CPE 471 Fall 2016 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>

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
    
    // Right now, hard coding these
    string meshName("../../resources/tri.OBJ");
    string imgName("out.PNG");

	//set g_width and g_height appropriately!
	g_width = g_height = 100;

   //create an image - an unsigned character array with 3
	auto image = make_shared<Image>(g_width, g_height);

    // TODO: Create a z-buffer image with the same dimensions as image. You'll use this to store the depth
    
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
	cout << "Vertices in posBuf: (" << posBuf.size()/3 << "): " << endl;
    for(int i = 0; i< posBuf.size()/3; i++){
        cout << i*3 << " : (" << posBuf[3* i] << ", " << posBuf[3* i + 1] << ", " << posBuf[3* i + 2] << ") "<< endl;
    }
    
    
	cout << "Triangles in triBuf (" << triBuf.size()/3 << "): " << endl;
    for(int i = 0; i< triBuf.size()/3; i++){
        cout << i*3 << " : (" << triBuf[3* i] << ", " << triBuf[3* i + 1] << ", " << triBuf[3* i + 2] << ") "<< endl;
    }

	//TODO add code to iterate through each triangle and rasterize it 
	
    
    // First, find the bounding box of all the obj coordinates in coordinate space
    double maxX;
    double maxY = NULL;
    double minX = NULL;
    double minY = NULL;

    // Iterate through the posBuf array of coordinates and pick the max and min for X and Y
    for(int i = 0; i< posBuf.size()/3; i++){
        if(!maxX || maxX < posBuf[3*i]) {
            maxX = posBuf[3*i];
        }
        // If for minX
        if(!minX || minX > posBuf[3*i]) {
            minX = posBuf[3*i];
        }
        // If for maxY
        if(!maxY || maxY < posBuf[3*i + 1]) {
            maxY = posBuf[3*i + 1];
        }
        if(!minY || minY > posBuf[3*i + 1]) {
            minY = posBuf[3*i + 1];
        }
    }
    // Print them out
    cout << "New maxX set to:" << maxX << endl;
    cout << "New minX set to:" << minX << endl;
    cout << "New maxY set to:" << maxY << endl;
    cout << "New minY set to:" << minY << endl;
    
    // Calculate a scale factor between the coordiate space and the 2D image
    double objLength = abs(maxX) + abs(minX); // Get the total X distance of the object
    double objHeight = abs(maxY) + abs(minY); // Get the total Y distance of the object
    
    double xScaleFactor = g_width / objLength;
    double yScaleFactor = g_height / objHeight;
    
    double scaleFactor = MIN(xScaleFactor, yScaleFactor);
    
    // Create a lower bounding pixel
    struct Point lowerBoundingPoint;
    lowerBoundingPoint.x = minX;
    lowerBoundingPoint.y = minY;
    
    Pixel lowerBoundingPixel = mapPointToPixel(lowerBoundingPoint, scaleFactor);
    cout << "The lowerBoundingPixel:" << " (" << lowerBoundingPixel.x << "," << lowerBoundingPixel.y << ")" << endl;
    image -> setPixel(lowerBoundingPixel.x, lowerBoundingPixel.y, 0, 255, 0);
    
    // Create an upper bounding pixel
    struct Point upperBoundingPoint;
    upperBoundingPoint.x = maxX;
    upperBoundingPoint.y = maxY;
    
    Pixel upperBoundingPixel = mapPointToPixel(upperBoundingPoint, scaleFactor);
    cout << "The upperBoundingPixel:" << " (" << upperBoundingPixel.x << "," << upperBoundingPixel.y << ")" << endl;
    image -> setPixel(upperBoundingPixel.x, upperBoundingPixel.y, 0, 255, 0);
    

    
    
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
        Pixel aPixel = mapPointToPixel(aPoint, scaleFactor);
        cout << "Created a NEW PIXEL:" << " (" << aPixel.x << "," << aPixel.y << ")" << endl;
//        image -> setPixel(aPixel.x, aPixel.y, 255, 0, 0);
        
        struct Point bPoint;
        bPoint.x = posBuf[3* pointB];
        bPoint.y = posBuf[3* pointB + 1];
        bPoint.z = posBuf[3* pointB + 2];
        Pixel bPixel = mapPointToPixel(bPoint, scaleFactor);
        cout << "Created a NEW PIXEL:" << " (" << bPixel.x << "," << bPixel.y << ")" << endl;
//        image -> setPixel(bPixel.x, bPixel.y, 255, 0, 0);
        
        struct Point cPoint;
        cPoint.x = posBuf[3* pointC];
        cPoint.y = posBuf[3* pointC + 1];
        cPoint.z = posBuf[3* pointC + 2];
        Pixel cPixel = mapPointToPixel(cPoint, scaleFactor);
        cout << "Created a NEW PIXEL:" << " (" << cPixel.x << "," << cPixel.y << ")" << endl;
//        image -> setPixel(cPixel.x, cPixel.y, 255, 0, 0);
        
        
        
        // Now draw the triangle by iterating through each pixel inside the bounding box and checking if it lies inside the triangle formed by aPoint, bPoint, and cPoint
        
        for(int x = lowerBoundingPixel.x; x < upperBoundingPixel.x; x++){
            for(int y = lowerBoundingPixel.y; y < upperBoundingPixel.y; y++) {
                
                struct Pixel currentPix;
                currentPix.x = x;
                currentPix.y = y;
                
                if (PointInTriangle(currentPix, aPixel, bPixel, cPixel)){
                    image -> setPixel(x, y, 255, 0, 0);
                }
                
            }
        }

        
    }
    
    
    
    // Go through each pixel in the bounding box and check it it's inside the triangle
    
    // If it is, create inner triangles connecting the current pixel to the three vertices of the outer triangle, then calculate the area of each inner triangle, then use the areas to calculate the values for the current pixel

    
    image -> setPixel(99, 99, 255, 0, 0);
    

    
	//write out the image
   image->writeToFile(imgName);

	return 0;
}
