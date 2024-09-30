## Threshold MSER In Situ Plankton Segmentation

Copyright © 2024 Oregon State University

Dominic W. Daprano
Sheng Tse Tsai
Moritz S. Schmid
Christopher M. Sullivan
Robert K. Cowen
Grant A. Towers

Hatfield Marine Science Center
Center for Qualitative Life Sciences
Oregon State University
Corvallis, OR 97331

This is a contribution to World Wide Web of Plankton Image Curation [wwwPIC](https://sites.google.com/view/wwwpic/home), a project funded by the Belmont Forum and the National Science Foundation (NSF award number 1927710). 

This program is described in Panaïotis et al. 2022:
Panaïotis T, Caray–Counil L, Woodward B, Schmid MS, Daprano D, Tsai ST, Sullivan CM, Cowen RK and Irisson J-O (2022) 
Content-Aware Segmentation of Objects Spanning a Large Size Range: Application to Plankton Images. Front. Mar. Sci. 9:870005. doi: 10.3389/fmars.2022.870005 
https://www.frontiersin.org/articles/10.3389/fmars.2022.870005/full

This program is distributed WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

This program is distributed under the GNU GPL v 2.0 or later license.

Any User wishing to make commercial use of the Software must contact the authors or Oregon State University directly to arrange an appropriate license. Commercial use includes (1) use of the software for commercial purposes, including integrating or incorporating all or part of the source code into a product for sale or license by, or on behalf of, User to third parties, or (2) distribution of the binary or source code to third parties for use with a commercial product sold or licensed by, or on behalf of, User.

------

This tool allows for the segmentation of in situ footage of plankton imagery. The 
segment binary takes video files or images as inputs. The frames are flat-fielded 
using a vertical average and then regions of interest (ROIs) are extracted with 
the Maximal Stable Extrema Regions (MSER) algorithm. ROIs are segmented and their 
information saved (area, major_axis, minor_axis, etc) as the output of the program.

#### Setup

In order to build the project CMake 3.0.0, C++17, OpenCV 4.0.0, and OpenMP are all required.

This project has been successfully compiled with g++ 8.0+ on Ubuntu 20.04 and Centos 8.

To setup, first clone the repository.

```
cd Threshold-MSER
mkdir build
cd build
cmake ../
make
```
Now the binary can be run.

```
./segment --help

Expected Output:
  -V, --verbose               Enable Verbose Mode (Default: 0)
  -VV, --verbosePlus          Enable VerbosePlus Mode [More VERBOSE] (Default: 0)
  -i, --input                 Directory of video files to segment
  -o, --output-directory      Output directory where segmented images should be stored (Default: out)
  -n, --num-concatenate       The number of frames that will be vertically concatenated (Default: 1)
  -s, --signal-to-noise       The cutoff signal to noise ratio that is used in determining which frames from
                              the video file get segmented. Note: This will change as we change the outlier percent (Default: 60)
  -p, --outlier-percent       Percentage of darkest and lightest pixels to throw out before flat-fielding (Default: 0.15)
  -M, --maxArea               Maximum area of a segmented blob (Default: 400000)
  -m, --minArea               Minimum area of a segmented blob. (Default: 50)
  -d, --delta                 Delta is a parameter for MSER. Delta is the number of steps (changes
                              in pixel brightness) MSER uses to compare the size of connected regions.
                              A smaller delta will produce more segments. (Default: 4)
  -v, --variation             Maximum variation of the region's area between delta threshold.
                              Larger values lead to more segments. (Default: 100)
  -e, --epsilon               Float between 0 and 1 that represents the maximum overlap between
                              two rectangle bounding boxes. 0 means that any overlap will mean
                              that the bounding boxes are treated as the same. (Default: 1)
  -t, --threshold             Value to threshold the images for low signal to noise images 
                              (Default: 160)
  -f, --full-ouput            If flag is included a directory of full frames is added to output
  -l, --left-crop             Crop this many pixels off of the left side of the image
  -r, --right-crop            Crop this many pixels off of the right side of the image
  -pip, --pipeline            Used to change how the files are saved to better work with the pipeline
                              (Default: 1)
  -O, --origin-img            Include the origional image to be used for writing boxes on frame(used if you want to export data to Njobvu) or (used in conjunction with the -f flag if you want to write boxes on the original image

```

####  Build Options

There are a few build options that can also be specified.  

OpenMP can be disabled with the WITH_OPENMP=OFF flag in cmake.
```
cmake -DWITH_OPENMP=OFF ../
```

Additionally, the segmentation process can be observed by using the VISUAL_MODE=ON flag in cmake.
*If the project is built with VISUAL_MODE=ON it will automatically set WITH_OPENMP=OFF.*
```
cmake -DVISUAL_MODE=ON ../
```

#### Quick start

The only required parameter of segment is the input parameter. This can either be a path to a 
directory containing video files, a path to a video file, or a path to a folder of images.
```
./segment -i <path/to/dir>
```

The segments that are produced can be controlled through several parameters. Most
notably, the --minium and --maximum paraters control the minimum and 
maximum size of a crop that can be extracted from a frame.

