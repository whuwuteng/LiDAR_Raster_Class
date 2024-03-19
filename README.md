# LiDARHD to raster image

## Use the sampling to create raster image with classification property

To raster the LiDAR point cloud, to save the memory, we can only save one label for each pixel, because **building** is the important part in the project (GENESE). So if in the pixel, there is a building point, we set this pixel to building label. For the other class, we save the last label. 

The out put file is a **Tiff file with tfw**, so that this file can have the geo-information, and this can be open with **QGIS**, so that the output can be overlap with the other data. 

After discussing with Bruno, we think it's better to use a priority and z value to select the class, so the priority should be set, the class should  set a positive value for the priority. (This first version is equal to building is 1 and the other classes are 0).

## Introduction

The class in LiDARHD, detail can find in the [document](/doc/DC_LiDAR_HD_1-0_PTS.pdf) :

| Valeur | Description|
| :----------: | :-----------: |
|1 | Non classé|
|2 | Sol|
|3 | Végétation basse (0-50 cm)|
|4| Végétation moyenne (50 cm-1,50 m)|
|5| Végétation haute (+1,50 m)|
|6|Bâtiment|
|9| Eau|
|17| Tablier de pont|
|64| Sursol pérenne|
|65| Artefacts|
|66| Points virtuels|
|67| Divers - bâtis|

~~For class 66, "Seuls les « nuages de points optimisés » contiennent des points classés avec cette valeur.", so at present, this class is not shown in the image.~~

For the class **64-67**, the color bar depends on the QGIS label, so the color probably not  same with QGIS.

## Dependency

The code depends on my code projects :

### [OpenImageoiioLib](https://github.com/whuwuteng/OpenImageoiioLib)

Because the code depends on  [OpenImageIO](https://sites.google.com/site/openimageio/home) , because we only write Tiff format, so it is easy to install.

```
# libtiff
sudo apt-get update && apt-get install -y \
		    build-essential \
		    make \ 
            git \
			automake \
			libtool \
			zlib1g-dev \
			libboost-all-dev \
			libtiff-dev \
			libpng-dev \
			libjpeg-dev \
			libeigen3-dev

# code from OpenImageIO
git clone --recurse https://github.com/OpenImageIO/oiio.git && \
    cd oiio && \
    mkdir build && \
    cd build && \
    cmake -DOIIO_BUILD_TESTS=0 -DUSE_OPENGL=0 -DUSE_QT=0 -DUSE_PYTHON=0 -DUSE_FIELD3D=0 -DUSE_FFMPEG=0 -DUSE_OPENJPEG=1 -DUSE_OPENCV=0 -DUSE_OPENSSL=0 -DUSE_PTEX=0 -DUSE_NUKE=0 -DUSE_DICOM=0 ../ && \
    make -j16 && 
    sudo make install
```

### [WuLasLib](https://github.com/whuwuteng/WuLasLib)

This part is already in the code repository, so it doesn't matter.

## Example

It's easy to use the code, the comande line is :

```
EXE LAS GSD TIF (WEIGHT)
```

If there is no **WEIGHT** file, so the max Z is used in the class priority.

 There is an example using the LiDARHD :

```
 cd LiDAR_Raster_Class
 mkdir build
 cd build
 cmake ..
 make -j4
 
 # EXE LAS GSD TIF
 ./ConvertLasImage '/home/whuwuteng/AI4GEO-Toulouse-IMAGE/ALS/LHD_FXX_0574_6277_PTS_C_LAMB93_IGN69.copc.laz' 0.15 '/home/whuwuteng/AI4GEO-Toulouse-IMAGE/ALS/raster2.tif' ../weight.txt
```

The example is shown (convert to png) :

| <img src="/figures/raster2.png" width="700" alt="*Raster image*" /> |
| :----------------------------------------------------------: |
|                *Raster image*                |


## MAINTENANCE
If you think you have any problem, contact [Teng Wu]<whuwuteng@gmail.com>