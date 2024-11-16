
#include <stdio.h>
#include <string.h>

#include <math.h>

// refer to https://stackoverflow.com/questions/55474690/stdfilesystem-has-not-been-declared-after-including-experimental-filesystem
//#include <filesystem>
//#include <experimental/filesystem>
#ifndef __has_include
  static_assert(false, "__has_include not supported");
#else
#  if __cplusplus >= 201703L && __has_include(<filesystem>)
#    include <filesystem>
     namespace fs = std::filesystem;
#  elif __has_include(<experimental/filesystem>)
#    include <experimental/filesystem>
     namespace fs = std::experimental::filesystem;
#  elif __has_include(<boost/filesystem.hpp>)
#    include <boost/filesystem.hpp>
     namespace fs = boost::filesystem;
#  endif
#endif
#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>

#include "WuLasLib.h"
#include "OpenImageoiio.h"

// for the double, avoid to use ==
const double EPS = 1.0e-4;
unsigned char BUILDING = 6;
const float NaN = -9999;

// set the priority file
bool LoadFilePriority(const char * pszFile, double * pPriority)
{
    std::fstream infile(pszFile);
    if (! infile){
        std::cout << "Can not open the file : " << pszFile << std::endl;
        return false;
    }
    
    std::string line;
    while (std::getline(infile, line)){
        std::istringstream iss(line);
        int nClass = 0;
        double Weight = 0.0;
        if (!(iss >> nClass >> Weight)) { 
            break; 
        }
        if (nClass > 0 && nClass < 256){
            pPriority[nClass] = Weight;
        }
    }
    return true;
}

// write a class selection
// but the class is not continuous
void UpdateWithPriority(unsigned char * pOldClass, double * pOldZ, double * pPriority,  unsigned char newClass, double newZValue)
{
    // no point in this pixel
    if (* pOldClass == 0){
        * pOldClass = newClass;
        * pOldZ = newZValue;
    }
    else {
        if (pPriority[newClass] > pPriority[* pOldClass]){
            * pOldClass = newClass;
            * pOldZ = newZValue;
        }
        else if (fabs(pPriority[newClass] - pPriority[* pOldClass]) < EPS){
            if (* pOldZ < newZValue){
                * pOldClass = newClass;
                * pOldZ = newZValue;
            }
        }
    }
}

// for the "projet de recherche"
// a new version that only building is needed
// we need to produce a height image
int main(int argc, char const *argv[])
{
    if (argc != 4 && argc != 5){
        std::cout << "ConvertLasImage Las GSD Tif" << std::endl;
        std::cout << "If there is no weight, so the max Z is used." << std::endl;
        std::cout << "Or: " << std::endl;
        std::cout << "ConvertLasImage Las GSD Tif Weight" << std::endl;
        return -1;
    }

    char szSrcLas[512] = { 0 };
    strcpy(szSrcLas, argv[1]);

    // LiDARHD
	//double gridsize = 0.15;
    double gridsize = atof(argv[2]);
    
    // max is 255
    double Priority[256] = { 0 };
    
    if (argc == 5){
        LoadFilePriority(argv[4], Priority);
    }
    // the defaut is building
    else{
        // building is 6
        Priority[BUILDING] = 1.0;
    }

	CWuLasLib srcLas;
	srcLas.Open(szSrcLas);

	int nPoints = srcLas.GetPtNum();

    std::cout << "Point number: " << nPoints << std::endl;

	LasPointClass * pFullPoint = new LasPointClass[nPoints];
	memset(pFullPoint, 0, sizeof(LasPointClass) * nPoints);
    
    srcLas.ReadLas(pFullPoint, nPoints);
	srcLas.Close();
    
	double minx = pFullPoint[0].x;
    double maxx = pFullPoint[0].x;
    double miny = pFullPoint[0].y;
    double maxy = pFullPoint[0].y;

    for (int i = 0; i < nPoints; ++i){
		if (minx > pFullPoint[i].x) minx = pFullPoint[i].x;
        if (maxx < pFullPoint[i].x) maxx = pFullPoint[i].x;
        
        if (miny > pFullPoint[i].y) miny = pFullPoint[i].y;
        if (maxy < pFullPoint[i].y) maxy = pFullPoint[i].y;
    }

	minx -= 2 * gridsize;
	maxx += 2 * gridsize;
	miny -= 2 * gridsize;
	maxy += 2 * gridsize;

	int nCols = int((maxx - minx)/gridsize) + 1;
	int nRows = int((maxy - miny)/gridsize) + 1;

    std::cout << "Rows:" << nRows << std::endl;
    std::cout << "Cols:" << nCols << std::endl;

	unsigned char * pLabelClass = new unsigned char[nRows * nCols];
    memset(pLabelClass, 0, sizeof(unsigned char) * nRows * nCols);
    
    double * pZValue = new double[nRows * nCols];
    memset(pZValue, 0, sizeof(double) * nRows * nCols);
    
    for (int i = 0; i < nPoints; ++i){
		int x = int((pFullPoint[i].x - minx)/gridsize);
		int y = int((pFullPoint[i].y - miny)/gridsize);
        
        // system different
        y = nRows - 1 - y;
/*
        if (pLabelClass[y * nCols + x] != 6){
            pLabelClass[y * nCols + x] = (unsigned char)(pFullPoint[i].classification);
        }
*/
        int nIndex = y * nCols + x;
        UpdateWithPriority(pLabelClass + nIndex, pZValue + nIndex, Priority, pFullPoint[i].classification, pFullPoint[i].z);
    }
    //delete []pZValue;               pZValue = NULL;
    
    std::cout << "Convert to raster image..." << std::endl;

    unsigned char * pRasterImage = new unsigned char[nRows * nCols * 3];
	memset(pRasterImage, 0, sizeof(unsigned char) * nRows * nCols * 3);

	for (int i = 0; i < nRows; ++i){
		for (int j = 0; j < nCols; ++j){
			if (pLabelClass[i * nCols + j] > 0){
                switch (pLabelClass[i * nCols + j]){
                    case 1 :
                        pRasterImage[(i * nCols + j) * 3] = 170;
                        pRasterImage[(i * nCols + j) * 3 + 1] = 170;
                        pRasterImage[(i * nCols + j) * 3 + 2] = 170;
                        break;
                    case 2 :
                        pRasterImage[(i * nCols + j) * 3] = 170;
                        pRasterImage[(i * nCols + j) * 3 + 1] = 85;
                        pRasterImage[(i * nCols + j) * 3 + 2] = 0;
                        break;
                    case 3 :
                        pRasterImage[(i * nCols + j) * 3] = 0;
                        pRasterImage[(i * nCols + j) * 3 + 1] = 170;
                        pRasterImage[(i * nCols + j) * 3 + 2] = 170;
                        break;
                    case 4 :
                        pRasterImage[(i * nCols + j) * 3] = 85;
                        pRasterImage[(i * nCols + j) * 3 + 1] = 255;
                        pRasterImage[(i * nCols + j) * 3 + 2] = 85;
                        break;
                    case 5 :
                        pRasterImage[(i * nCols + j) * 3] = 0;
                        pRasterImage[(i * nCols + j) * 3 + 1] = 170;
                        pRasterImage[(i * nCols + j) * 3 + 2] = 0;
                        break;
                    case 6 :
                        pRasterImage[(i * nCols + j) * 3] = 255;
                        pRasterImage[(i * nCols + j) * 3 + 1] = 85;
                        pRasterImage[(i * nCols + j) * 3 + 2] = 85;
                        break;
                    case 17 :
                        pRasterImage[(i * nCols + j) * 3] = 85;
                        pRasterImage[(i * nCols + j) * 3 + 1] = 85;
                        pRasterImage[(i * nCols + j) * 3 + 2] = 255;
                        break;
                    case 64 :
                        pRasterImage[(i * nCols + j) * 3] = 114;
                        pRasterImage[(i * nCols + j) * 3 + 1] = 155;
                        pRasterImage[(i * nCols + j) * 3 + 2] = 111;
                        break;
                    case 65 :
                        pRasterImage[(i * nCols + j) * 3] = 213;
                        pRasterImage[(i * nCols + j) * 3 + 1] = 180;
                        pRasterImage[(i * nCols + j) * 3 + 2] = 60;
                        break;
                    case 66 :
                        pRasterImage[(i * nCols + j) * 3] = 141;
                        pRasterImage[(i * nCols + j) * 3 + 1] = 90;
                        pRasterImage[(i * nCols + j) * 3 + 2] = 153;
                        break;
                    case 67 :
                        pRasterImage[(i * nCols + j) * 3] = 164;
                        pRasterImage[(i * nCols + j) * 3 + 1] = 113;
                        pRasterImage[(i * nCols + j) * 3 + 2] = 88;
                        break;
                    default :
                        pRasterImage[(i * nCols + j) * 3] = 255;
                        pRasterImage[(i * nCols + j) * 3 + 1] = 255;
                        pRasterImage[(i * nCols + j) * 3 + 2] = 255;
                }
            }
			else{
                pRasterImage[(i * nCols + j) * 3] = 255;
                pRasterImage[(i * nCols + j) * 3 + 1] = 255;
                pRasterImage[(i * nCols + j) * 3 + 2] = 255;
            }
		}
	}
	
	//delete []pLabelClass;                  pLabelClass = NULL;

	// write the geotif file
	std::cout << "Save image..." << std::endl;
    
    char szTarTif[512] = { 0 };
    strcpy(szTarTif, argv[3]);
    
    SaveImg(szTarTif, pRasterImage, nRows, nCols, 3);
    delete []pRasterImage;                  pRasterImage = NULL;

    char szHeightTif[512] = { 0 };
    strcpy(szHeightTif, argv[3]);
    strcpy(strrchr(szHeightTif, '.'), "_height.tif");

    // only keep the building and the other is 0
    float * pHeight = new float[nRows * nCols];
    memset(pHeight, 0, sizeof(float) * nRows * nCols);
    for (int i = 0; i < nRows; ++i){
		for (int j = 0; j < nCols; ++j){
            if (pLabelClass[i * nCols + j] == BUILDING){
                pHeight[i * nCols + j] = pZValue[i * nCols + j];
            }
            else{
                pHeight[i * nCols + j] = NaN;
            }
        }
    }
    delete []pLabelClass;                  pLabelClass = NULL;
    delete []pZValue;                      pZValue = NULL;

    SaveImg(szHeightTif, pHeight, nRows, nCols, true);
    delete []pZValue;                      pHeight = NULL;
    
    // save the TFW
    char szTifTFW[512] = { 0 };
    strcpy(szTifTFW, szTarTif);
    strcpy(strrchr(szTifTFW, '.'), ".tfw");
    
    FILE * fp = fopen(szTifTFW, "w");
    fprintf(fp, "%lf\n%lf\n%lf\n%lf\n%lf\n%lf\n", gridsize, 0.0, 0.0, -gridsize, minx, miny + nRows * gridsize);
    fclose(fp);

    char szTifTFW2[512] = { 0 };
    strcpy(szTifTFW2, szHeightTif);
    strcpy(strrchr(szTifTFW2, '.'), ".tfw");

    //std::filesystem::copy(szTifTFW, szTifTFW2, std::filesystem::copy_options::overwrite_existing);
    fs::copy(szTifTFW, szTifTFW2, fs::copy_options::overwrite_existing);
	return 0;
}
