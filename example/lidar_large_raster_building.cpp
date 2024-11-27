
#include <stdio.h>
#include <string.h>

#include <math.h>

#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>

#include "WuLasLib.h"
#include "OpenImageoiio.h"

// for the double, avoid to use ==
const double EPS = 1.0e-4;
unsigned char BUILDING = 6;

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
	//srcLas.Close();

    // make sure that the LAS file is correctly written
/*    
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

    std::cout << "Min X : " << minx << " Max X : " << maxx << std::endl;
    std::cout << "Min Y : " << miny << " Max Y : " << maxy << std::endl;
*/
    
    double minx = 0;
    double maxx = 0;
    double miny = 0;
    double maxy = 0;
    double minz = 0;
    double maxz = 0;
    srcLas.GetRange(minx, maxx, miny, maxy, minz, maxz);
    srcLas.Close();

    std::cout << "Min X : " << minx << " Max X : " << maxx << std::endl;
    std::cout << "Min Y : " << miny << " Max Y : " << maxy << std::endl;
 
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
    delete []pFullPoint;                  pFullPoint = NULL;
    delete []pZValue;               pZValue = NULL;
    
    std::cout << "Convert to raster image..." << std::endl;

    // write the geotif file
	std::cout << "And save image sametime..." << std::endl;

    char szTarTif[512] = { 0 };
    strcpy(szTarTif, argv[3]);

    COpenImageIO writeImg;
    writeImg.Open(szTarTif, COpenImageIO::modeCreate);
    writeImg.SetRows(nRows);
    writeImg.SetCols(nCols);
    writeImg.SetPixelBytes(3);
    writeImg.SetDataType(COpenImageIO::UCHAR);

    unsigned char * pRasterImage = new unsigned char[nCols * 3];
	memset(pRasterImage, 0, sizeof(unsigned char) * nCols * 3);

	for (int i = 0; i < nRows; ++i){
		for (int j = 0; j < nCols; ++j){
			if (pLabelClass[i * nCols + j] == BUILDING){
                pRasterImage[j * 3] = 255;
                pRasterImage[j * 3 + 1] = 85;
                pRasterImage[j * 3 + 2] = 85;
            }
			else{
                pRasterImage[j * 3] = 255;
                pRasterImage[j * 3 + 1] = 255;
                pRasterImage[j * 3 + 2] = 255;
            }
		}
        writeImg.Write(pRasterImage, i);
	}
	
	delete []pLabelClass;                  pLabelClass = NULL;
    delete []pRasterImage;                 pRasterImage = NULL;
    
    // save the TFW
    char szTifTFW[512] = { 0 };
    strcpy(szTifTFW, szTarTif);
    strcpy(strrchr(szTifTFW, '.'), ".tfw");
    
    FILE * fp = fopen(szTifTFW, "w");
    fprintf(fp, "%lf\n%lf\n%lf\n%lf\n%lf\n%lf\n", gridsize, 0.0, 0.0, -gridsize, minx, miny + nRows * gridsize);
    fclose(fp);
	return 0;
}
