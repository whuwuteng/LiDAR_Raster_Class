# LiDARHD to raster image

## Use the sampling to create raster image with classification property

To raster the LiDAR point cloud, to save the memory, we can only save one label for each pixel, because **building** is the important part in the project (GENESE). So if in the pixel, there is a building point, we set this pixel to building label. For the other class, we save the last label. 

The out put file is a **Tiff file with tfw**, so that this file can have the geo-information, and this can be open with **QGIS**, so that the output can be overlap with the other data. 

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
EXE LAS GSD TIF
```

 there is an example using the LiDARHD :

```
 cd LiDAR_Raster_Class
 mkdir build
 cd build
 cmake ..
 make -j4
 
 # EXE LAS GSD TIF
 ./ConvertLasImage '/home/whuwuteng/AI4GEO-Toulouse-IMAGE/ALS/LHD_FXX_0574_6277_PTS_C_LAMB93_IGN69.copc.laz' 0.15 '/home/whuwuteng/AI4GEO-Toulouse-IMAGE/ALS/raster2.tif'
```

The example is shown (convert to png) :

| <img src="/figures/raster2.png" width="700" alt="*Raster image*" /> |
| :----------------------------------------------------------: |
|                *Raster image*                |


## MAINTENANCE
If you think you have any problem, contact [Teng Wu]<whuwuteng@gmail.com>