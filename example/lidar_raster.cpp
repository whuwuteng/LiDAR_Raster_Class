
#include <stdio.h>
#include <string.h>

#include <math.h>

#include <iostream>
#include <cstdio>
#include <vector>

#include "WuLasLib.h"
#include "OpenImageoiio.h"

// compare to https://www.cloudcompare.org/doc/wiki/index.php/Rasterize
// for TFW file gdal_translate -co "TFW=YES"  raster.tif raster2.tif
int main(int argc, char const *argv[])
{
	char szSrcLas[512] = "/home/whuwuteng/AI4GEO-Toulouse-IMAGE/ALS/LHD_FXX_0574_6277_PTS_C_LAMB93_IGN69.copc.laz";

	double gridsize = 0.15;

	CWuLasLib srcLas;
	srcLas.Open(szSrcLas);

	int nPoints = srcLas.GetPtNum();

    std::cout << "Point number: " << nPoints << std::endl;
    // keep as much as information
	LasPointClass * pFullPoint = new LasPointClass[nPoints];
	memset(pFullPoint, 0, sizeof(LasPointClass) * nPoints);
    
    srcLas.ReadLas(pFullPoint, nPoints);
	srcLas.Close();

    //return -1;
    
    // debug
    // nPoints = 10000;
    
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

    // we can use a 2D version, but the 3D version is already exist
	unsigned char * pLabelClass = new unsigned char[nRows * nCols];
    memset(pLabelClass, 0, sizeof(unsigned char) * nRows * nCols);
    
    // image and point cloud the system is different
    // refer to http://geotiff.maptools.org/spec/geotiff2.5.html
    for (int i = 0; i < nPoints; ++i){
		int x = int((pFullPoint[i].x - minx)/gridsize);
		int y = int((pFullPoint[i].y - miny)/gridsize);
        
        // system different
        y = nRows - 1 - y;
        if (pLabelClass[y * nCols + x] != 6){
            pLabelClass[y * nCols + x] = (unsigned char)(pFullPoint[i].classification);
        }
    }
    
    std::cout << "Convert to raster image..." << std::endl;
    // refer to https://docs.qgis.org/3.34/en/docs/user_manual/working_with_point_clouds/point_clouds.html
    // set the point cloud classification renderer
    // generate the image
    // 1 170 170 170
    // 2 170 85 0
    // 3 0 170 170
    // 4 85 255 85
    // 5 0 170 0
    // 6 255 85 85
    // 17 85 85 255
    // 64 232 113 141
    // 65 141 90 153
    // 67 243 166 178
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
	
	delete []pLabelClass;                  pLabelClass = NULL;

	// write the geotif file
	std::cout << "Save image..." << std::endl;
	char szTarTif[512] = "/home/whuwuteng/AI4GEO-Toulouse-IMAGE/ALS/LHD_FXX_0574_6277_PTS_C_LAMB93_IGN69.copc.tif";
    
    SaveImg(szTarTif, pRasterImage, nRows, nCols, 3);
    delete []pRasterImage;                  pRasterImage = NULL;
    
    // save the TFW
    char szTifTFW[512] = { 0 };
    strcpy(szTifTFW, szTarTif);
    strcpy(strrchr(szTifTFW, '.'), ".tfw");
    
    FILE * fp = fopen(szTifTFW, "w");
    fprintf(fp, "%lf\n%lf\n%lf\n%lf\n%lf\n%lf\n", gridsize, 0.0, 0.0, -gridsize, minx, miny + nRows * gridsize);
    fclose(fp);
	return 0;
}
