/** @file main.cpp
 *
 *  Copyright © 2022 Oregon State University
 *
 *  Dominic W. Daprano
 *  Sheng Tse Tsai 
 *  Moritz S. Schmid
 *  Christopher M. Sullivan
 *  Robert K. Cowen
 *  Grant A. Towers
 *
 *  Hatfield Marine Science Center
 *  Center for Qualitative Life Sciences
 *  Oregon State University
 *  Corvallis, OR 97331
 *
 *  This program is distributed WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * 
 *  This program is distributed under the GNU GPL v 2.0 or later license.
 *
 *  Any User wishing to make commercial use of the Software must contact the authors 
 *  or Oregon State University directly to arrange an appropriate license.
 *  Commercial use includes (1) use of the software for commercial purposes, including 
 *  integrating or incorporating all or part of the source code into a product 
 *  for sale or license by, or on behalf of, User to third parties, or (2) distribution 
 *  of the binary or source code to third parties for use with a commercial 
 *  product sold or licensed by, or on behalf of, User.
 *
 */ 


//GRANT NEED TO FIX PROBLEM WHERE THE IMG NAME IS INCORRECT DUE TO TRUNCATING THE FILE EXT
//TODO Currently working on changing file structure to better fit the pipeline model while keeping what chris made changes too as a flag
#include <iostream>
#include <fstream> // write output csv files
#include <iomanip>  // For the function std::setw

#include <filesystem>

#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "imageProcessing.hpp"

#if defined(WITH_OPENMP) and !defined(WITH_VISUAL)
    #include "omp.h" // OpenMP
#endif

namespace fs = std::filesystem;


void helpMsg(std::string executable, Options options) {
    std::cout << "Usage: " << executable << " -i <input> [OPTIONS]\n"
        << std::left << std::setw(30) << "Segment a directory of images by utilizing the MSER method.\n\n"

	<< std::left << std::setw(30) << "  -V, --verbose"<<" Enable Verbose Mode (Default: "<<options.verboseMode<<")\n"

	<< std::left << std::setw(30) << "  -VV, --verbosePlus"<<" Enable VerbosePlus Mode [More VERBOSE] (Default: "<<options.verboseModePlus<<")\n"


        << std::left << std::setw(30) << "  -i, --input" << "Directory of video files to segment\n"

        << std::left << std::setw(30) << "  -o, --output-directory" << "Output directory where segmented images should be stored (Default: " << options.outputDirectory << ")\n"

        << std::left << std::setw(30) << "  -n, --num-concatenate" << "The number of frames that will be vertically concatenated (Default: " << options.numConcatenate <<  ")\n"

        << std::left << std::setw(30) << "  -s, --signal-to-noise" << "The cutoff signal to noise ratio that is used in determining which frames from\n"
        << std::left << std::setw(30) << "" << "the video file get segmented. Note: This will change as we change the outlier percent (Default: " << options.signalToNoise << ")\n"

        << std::left << std::setw(30) << "  -p, --outlier-percent" << "Percentage of darkest and lightest pixels to throw out before flat-fielding (Default: " << options.outlierPercent << ")\n"

        << std::left << std::setw(30) << "  -M, --maxArea" << "Maximum area of a segmented blob (Default: " << options.maxArea << ")\n" 

        << std::left << std::setw(30) << "  -m, --minArea" << "Minimum area of a segmented blob. (Default: " << options.minArea << ")\n"

        << std::left << std::setw(30) << "  -d, --delta" << "Delta is a parameter for MSER. Delta is the number of steps (changes\n"
        << std::left << std::setw(30) << "" << "in pixel brightness) MSER uses to compare the size of connected regions.\n" 
        << std::left << std::setw(30) << "" <<  "A smaller delta will produce more segments. (Default: " << options.delta << ")\n"

        << std::left << std::setw(30) << "  -v, --variation" << "Maximum variation of the region's area between delta threshold.\n"
        << std::left << std::setw(30) << "" <<  "Larger values lead to more segments. (Default: " << options.variation << ")\n" 

        << std::left << std::setw(30) << "  -e, --epsilon" << "Float between 0 and 1 that represents the maximum overlap between\n"
        << std::left << std::setw(30) << "" << "two rectangle bounding boxes. 0 means that any overlap will mean\n"
        << std::left << std::setw(30) << "" << "that the bounding boxes are treated as the same. (Default: " << options.epsilon << ")\n"

        << std::left << std::setw(30) << "  -t, --threshold" << "Value to threshold the images for low signal to noise images \n"
        << std::left << std::setw(30) << "" <<  "(Default: " << options.threshold << ")\n" 

        << std::left << std::setw(30) << "  -f, --full-ouput" << "If flag is included a directory of full frames is added to output\n"

        << std::left << std::setw(30) << "  -l, --left-crop" << "Crop this many pixels off of the left side of the image\n"

        << std::left << std::setw(30) << "  -r, --right-crop" << "Crop this many pixels off of the right side of the image\n"

        << std::left << std::setw(30) << "  -pip, --pipeline" << "Used to change how the files are saved to better work with the pipeline\n"
        << std::left << std::setw(30) << "" <<  "(Default: " << options.pipeline<< ")\n" 

	//maybe just have a script that runs the segmentation
        //<< std::left << std::setw(30) << " -N, --njobvu" << "Create New NJobvu project with the values output from segmentaion (used in conjunction with the -O flag)\n"

        << std::left << std::setw(30) << "  -O, --origin-img" << "Include the origional image to be used for writing boxes on frame(used if you want to export data to Njobvu) or (used in conjunction with the -f flag if you want to write boxes on the original image" << std::endl;

}

int main(int argc, char **argv) {
    // Set the default options
	// JUDST ADD A VERBOSE MODE OPTIION COPY FULL OUTPUT
    Options options;
    options.input = "";
    options.outputDirectory = "out";
    options.signalToNoise = 60;
    options.outlierPercent = .15;
    options.numConcatenate = 1;
    options.minArea = 50;
    options.maxArea = 400000;
    options.epsilon = 1;
    options.delta = 4;
    options.variation = 100;
    options.threshold = 160;
    options.fullOutput = false;
    options.left = 0;
    options.right = 0;
    options.verboseMode = false;
    options.verboseModePlus = false;
    options.ogImg = "";
    options.pipeline= true;

    // TODO: more robust options with std::find may be worth it
    if (argc == 1) {
        helpMsg(argv[0], options);
        // Print the number of threads that will be used by this program
        #pragma omp parallel
        {
            #pragma omp single
            {
                #if defined(WITH_OPENMP)
                int nthreads = omp_get_num_threads();
                std::cout << "OMP Num Threads: " << nthreads << std::endl;
                #endif
            }
        }
    }

    int i = 1;
	while (i < argc) {
        // Display the help message
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
        		helpMsg(argv[0], options);

			return 0;
        	}
		else if(strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--verbose") == 0){
            		// toggle
            		options.verboseMode = true; 

            		i+=1;
		}
		else if(strcmp(argv[i], "-VV") == 0 || strcmp(argv[i], "--verbosePlus") == 0){
            		// toggle
            		options.verboseModePlus = true; 

            		i+=1;
		}

		else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
            		options.input = argv[i + 1];
            		if ( !fs::exists(options.input) ) {
                		std::cerr << options.input << " does not exist." << std::endl;
                		return 1;
            		}
            		i+=2;
        	} 
		else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output-directory") == 0) {
            		options.outputDirectory = argv[i + 1];
            		try {
                		fs::create_directories(options.outputDirectory);
            		}
            		catch(fs::filesystem_error const& ex){
                		std::cerr << ex.what() <<  std::endl;
                		return 1;
            		}
            		i+=2;
		} 
		else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--signal-to-noise") == 0) {
            		// Validate the input type
            		if ( !isInt(argv[i+1]) ) {
                	std::cerr << argv[i+1] << " is not a valid input. Signal To Noise ration must be an integer." << std::endl;
                	return 1;
            		}
            		options.signalToNoise = std::stoi(argv[i+1]);
            		i+=2;
		} 
		else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--num-concatenate") == 0) {
            		// Validate the input type
            		if ( !isInt(argv[i+1]) ) {
                		std::cerr << argv[i+1] << " is not a valid input. The number of frames to concatenate must be an integer." << std::endl;
                		return 1;
            		}
            		options.numConcatenate = std::stoi(argv[i+1]);
            		i+=2;
		} 
		else if (strcmp(argv[i], "-M") == 0 || strcmp(argv[i], "--maxArea") == 0) {
            		// Validate the input type
            		if ( !isInt(argv[i+1]) ) {
                		std::cerr << argv[i+1] << " is not a valid input. Maximum must be an integer." << std::endl;
                		return 1;
            		}
            		options.maxArea = std::stoi(argv[i+1]);
            		i+=2;
		} 
		else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--minArea") == 0) {
            		// Validate the input type
            		if ( !isInt(argv[i+1]) ) {
                	std::cerr << argv[i+1] << " is not a valid input. Minimum must be an integer." << std::endl;
                	return 1;
            		}
            		options.minArea = std::stoi(argv[i+1]);
            		i+=2;
		} 
		else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--epsilon") == 0) {
            		// Validate the input type
            		options.epsilon = std::stof(argv[i+1]); // FIXME: may throw error if not int

            		if (options.epsilon < 0) {
                		std::cerr << options.epsilon << " is not a valid input. Epsilon must be a non-negative float." << std::endl;
                		return 1;
            	}
            		i+=2;
		}
		else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--delta") == 0) {
        		// Validate the input type
            		if ( !isInt(argv[i+1]) ) {
                		std::cerr << argv[i+1] << " is not a valid input. Delta must be an integer." << std::endl;
                		return 1;
            		}
            		options.delta = std::stoi(argv[i+1]);
            		i+=2;

		} 
		else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--variation") == 0) {
        		// Validate the input type
            		if ( !isInt(argv[i+1]) ) {
                		std::cerr << argv[i+1] << " is not a valid input. Variation must be an integer." << std::endl;
                		return 1;
            		}
            		options.variation = std::stoi(argv[i+1]);
            		i+=2;
		} 
		else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--threshold") == 0) {
            		// Validate the input type
            		options.threshold = std::stof(argv[i+1]); // FIXME: may throw error if not int

            		if (options.threshold < 0 && options.threshold > 255) {
                		std::cerr << options.threshold << " is not a valid input. Threshold must be between 0 and 255 inclusive" << std::endl;
                		return 1;
            		}
            		i+=2;
		} 
		else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--outlier-percent") == 0) {
        		// Validate the input type
            		options.outlierPercent = std::stof(argv[i+1]); // FIXME: may throw error if not int

            		if (options.outlierPercent > 1 or options.outlierPercent < 0) {
                		std::cerr << options.outlierPercent << " is not a valid input. Outlier percent must be a float between 0 and 1." << std::endl;
                		return 1;
            	}
            		i+=2;
		} 
		else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--full-output") == 0) {
            		// If flag exists then add output
            		options.fullOutput = true; 

            		i+=1;
		} 
		else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--left-crop") == 0) {
            		// Validate the input type
            		if ( !isInt(argv[i+1]) ) {
                		std::cerr << argv[i+1] << " is not a valid input. Left crop must be a positive integer." << std::endl;
                		return 1;
            		} 
			else if (std::stoi(argv[i+1]) < 0) {
                		std::cerr << argv[i+1] << " is not a valid input. Left crop must be a positive integer." << std::endl;
                		return 1;
            		}

            		options.left = std::stoi(argv[i+1]);
            		i+=2;
		} 
		else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--right-crop") == 0) {
            		// Validate the input type
            		if ( !isInt(argv[i+1]) ) {
                		std::cerr << argv[i+1] << " is not a valid input. Right crop must be a positive integer." << std::endl;
                		return 1;
            		} 
			else if (std::stoi(argv[i+1]) < 0) {
                		std::cerr << argv[i+1] << " is not a valid input. Right crop must be a positive integer." << std::endl;
                		return 1;
            		}
            		options.right = std::stoi(argv[i+1]);
            		i+=2;
		} 
		else if (strcmp(argv[i], "-O") == 0 || strcmp(argv[i], "--origin-img") == 0) {
            		options.ogImg = argv[i + 1];
            		if ( !fs::exists(options.ogImg) ) {
                		std::cerr << options.ogImg<< " does not exist." << std::endl;
                		return 1;
            		}
            		i+=2;
        	} 
		else if (strcmp(argv[i], "-pip") == 0 || strcmp(argv[i], "--pipeline")==0){
			if(strcmp(argv[i+1],"true")==0){
				options.pipeline = true;
			}
			else{
				options.pipeline = false;
			}
		}
		else {
            		// Display invalid option message
            		std::cerr << argv[0] << ": invalid option \'" << argv[i] << "\'" <<
                	"\nTry \'" << argv[0] << " --help\' for more information." << std::endl;

            		return 1;
            		i+=2;
		}
	}
    if ( options.input == "" ) {
        std::cerr << "Must have either an input directory or an input file for segmentation to be run on" << std::endl;
        return 1;
    }



    if (options.verboseMode || options.verboseModePlus) {

		std::cout<<"*************************************************\n\n"<<std::endl;
		std::cout<<"Segmenting Using These Values:\n"<<std::endl;
		std::cout<<"Input          : "<<options.input<<"\n"<<std::endl;
		std::cout<<"Signal to Noise: "<<options.signalToNoise<<"\n"<<std::endl;
		std::cout<<"Outlier Percent: "<<options.outlierPercent<<"\n"<<std::endl;
		std::cout<<"numConcatenate : "<<options.numConcatenate<<"\n"<<std::endl;
		std::cout<<"minArea        : "<<options.minArea<<"\n"<<std::endl;
		std::cout<<"maxArea        : "<<options.maxArea<<"\n"<<std::endl;
		std::cout<<"epsilon        : "<<options.epsilon<<"\n"<<std::endl;
		std::cout<<"delta          : "<<options.delta<<"\n"<<std::endl;
		std::cout<<"variation      : "<<options.variation<<"\n"<<std::endl;
		std::cout<<"threshold      : "<<options.threshold<<"\n"<<std::endl;
		std::cout<<"full output    : "<<options.fullOutput<<"\n"<<std::endl;
		std::cout<<"left           : "<<options.left<<"\n"<<std::endl;
		std::cout<<"right          : "<<options.right<<"\n"<<std::endl;
		std::cout<<"color Image    : "<<options.ogImg<<"\n"<<std::endl;
		std::cout<<"pipeline       : "<<options.pipeline<<"\n"<<std::endl;
		std::cout<<"*************************************************\n\n"<<std::endl;
    }

	std::string measureDir;
	std::string segmentDir;
	std::string imgDir;
	if(options.pipeline){
    		// create output directories
		fs::create_directory(options.outputDirectory);
		measureDir = options.outputDirectory + "/measurements";
		fs::create_directory(measureDir);
		segmentDir = options.outputDirectory + "/segmentation";
		fs::create_directory(segmentDir);
 		if (options.verboseMode || options.verboseModePlus) {
			std::cout<<"Dir: "<<options.outputDirectory<<" Created\n"<<std::endl;
			std::cout<<"Dir: "<<measureDir<<" Created\n"<<std::endl;
			std::cout<<"Dir: "<<segmentDir<<" Created\n"<<std::endl;
			
		}
	}
    // Create vector of video files from the input which can either be a directory 
    // or a single avi file.

	std::vector<fs::path> color_files;
	if (fs::is_directory(options.ogImg)){
		for(auto& p: fs::directory_iterator{options.ogImg}){

			fs::path file(p);
			std::string ext = file.extension();
			std::string valid_ext[] = {".png", ".tif", ".jpg",".jpeg"};
			int len = sizeof(valid_ext)/sizeof(valid_ext[0]);
			if (!containExt(ext, valid_ext, len)){
				continue;
			}
			color_files.push_back(file);
		}
	}
	else{
		color_files.push_back(fs::path(options.ogImg));
	}
	int numColorFiles = color_files.size();
	if (numColorFiles < 1){
		if(options.verboseMode){
			std::cout<<"no original images found"<<std::endl;
		}
	}



    std::vector<fs::path> files;
    if (fs::is_directory(options.input)) {
        for(auto& p: fs::directory_iterator{options.input}) {
            fs::path file(p);

            std::string ext = file.extension();

            std::string valid_ext[] = {".avi", ".mp4", ".png"};
            int len = sizeof(valid_ext)/sizeof(valid_ext[0]);
            if (!containExt(ext, valid_ext, len)) {
                continue;
            }
            files.push_back(file);
        }
    } 
    else {
        files.push_back(fs::path(options.input));
    }

    int numFiles = files.size();
    if (numFiles < 1 ) {
        std::cerr << "No files to segment were found." << std::endl;
        return 1;
    }

    for (int i=0; i<numFiles; i++) {
	//CHRIS CODE add here
        fs::path file = files[i];
	fs::path color_file = color_files[i];
	std::string color_file_name = color_file.filename().string();
        std::string fileName = file.stem();
	std::string imgName;
	
	if(options.pipeline == false){
		std::string measureDirBase = options.outputDirectory + "/" + fileName;
		fs::create_directory(measureDirBase);
		measureDir = measureDirBase + "/measurements";
		fs::create_directory(measureDir);

		if (options.verboseMode) {
			std::cout<<"Dir: "<<measureDirBase<<" Created\n"<<std::endl;
			std::cout<<"Dir: "<<measureDir<<" Created\n"<<std::endl;
		}
    	}

	//create csv for bounding box data
	std::string bboxSheet = measureDir + "/" + fileName+"_bboxData.txt";
	std::ofstream bboxPtr(bboxSheet);
	//create txt file for yolo format bbox data
	std::string yoloFormatSheet = measureDir + "/" + fileName+"_yoloFormat.txt";
	std::ofstream yoloPtr(yoloFormatSheet);

        // Create a measurement file to save crop info to
        std::string measureFile = measureDir + "/" + fileName + ".csv";
        std::ofstream measurePtr(measureFile);
        measurePtr << "image,area,major,minor,perimeter,x,y,mean,height" << std::endl; 

        // TODO: Add a way to check if file is valid
        // FIXME: cap.read() and cap.grad() are not working properly, aren't throwing errors when reading image
        // This is a temporary solution to determine if the input file is an image or video 
        // cv::Mat testFrame;
        std::string ext = file.extension();
        bool validImage = (ext == ".png") || (ext == ".jpg") || (ext == ".jpeg") || (ext == ".tif") || (ext == ".gif");
	
	if (options.verboseMode && validImage) {
			std::cout<<"Your img "<<file<<" is valid\n"<<std::endl;
	}

        if (!validImage) { // If the file is a video
            cv::VideoCapture cap(file.string());
            if (!cap.isOpened()) {
                std::cerr << "Invalid file: " << file.string() << std::endl;
                continue;
            }

	    if (options.verboseMode) {
	    	std::cout<<"your video "<<file<<" is valid\n"<<std::endl;
	    }
	    int image_stack_counter = 0;
            int totalFrames = cap.get(cv::CAP_PROP_FRAME_COUNT);

            #pragma omp parallel for
			//CHRIS CODE add here
            for (int j=0; j<totalFrames-1; j++) 
            {   
	        	cv::Mat imgGray;
                #pragma omp critical(getImage)
                {
                    getFrame(cap, imgGray, options.numConcatenate);
                    image_stack_counter += options.numConcatenate;
                    j += options.numConcatenate - 1;
                }
		
		if(options.pipeline == false){
                	imgName = fileName + "_" + convertInt(image_stack_counter, 4);
			segmentDir = options.outputDirectory + "/" + fileName;
                	fs::create_directory(segmentDir);
			imgDir = segmentDir + "/segmentation";
			fs::create_directory(imgDir);
		}
		else{
			imgName = fileName + '_' + convertInt(image_stack_counter, 4);
			imgDir = segmentDir + "/" + fileName;

			fs::create_directory(imgDir);
		}
		if(options.verboseMode){
			std::cout<<"Dir: "<<imgDir<<" Created\n"<<std::endl;
		}

                int fill = fillSides(imgGray, options.left, options.right);
                if (fill != 0) {
                    exit( 1 );
                }

                cv::Mat imgCorrect;
                std::vector<cv::Rect> bboxes;
                segmentImage(imgGray, imgCorrect, bboxes, options);
		if(options.ogImg != ""){
                	saveCrops(imgGray, imgCorrect, bboxes, imgDir, imgName, measurePtr, options, bboxPtr, yoloPtr, color_file_name);
		}
		else{
                	saveCrops(imgGray, imgCorrect, bboxes, imgDir, imgName, measurePtr, options, bboxPtr, yoloPtr, "");
		}

                imgGray.release();
                imgCorrect.release();
	        }
	        // When video is done being processed release the capture object
	        cap.release();
        }
        else { // If the file is an image 
	    cv::Mat imgRaw = cv::imread(file.string());
            cv::Mat imgGray;
	    cv::cvtColor(imgRaw, imgGray, cv::COLOR_RGB2GRAY);

            if (imgGray.empty()) {
                std::cerr << "Error reading the image file " << file.string() << std::endl;
                continue;
            }
            // TODO: Add the ability to concatenate frames like with videos

	   //CHRIS CODE add here

	    if(options.pipeline == false){
            	std::string imgName = fileName;
            	//std::string imgDir = segmentDir + "/" + imgName;
    	    	std::string segmentDir = options.outputDirectory + "/" + fileName;
            	fs::create_directories(segmentDir);
	    	imgDir = segmentDir + "/segmentation";
	    	fs::create_directories(imgDir);

	    }
	    else{
		std::cout<<"segmentDir: "<<segmentDir<<"\n"<<std::endl;
		std::cout<<"fileName: "<<fileName<<"\n"<<std::endl;
		std::string imgName = fileName;
		imgDir = segmentDir + "/" + imgName;
		std::cout<<"ImgDir: "<<imgDir<<"\n"<<std::endl;
		fs::create_directories(imgDir);
	    }
            int fill = fillSides(imgGray, options.left, options.right);
            if (fill != 0) {
                exit( 1 );
            }
            
            // Segment the grayscale image and save its' crops.
            cv::Mat imgCorrect;
            std::vector<cv::Rect> bboxes;
	    
	    if (options.verboseMode || options.verboseModePlus) {
	    	std::cout<<"Segmenting..."<<std::endl;
	    }
            segmentImage(imgGray, imgCorrect, bboxes, options);
	    std::cout<<imgDir<<" idir\n"<<std::endl;
		if(options.ogImg != ""){

                	saveCrops(imgGray, imgCorrect, bboxes, imgDir, imgName, measurePtr, options, bboxPtr, yoloPtr, color_file_name);
		}
		else{
			
                	saveCrops(imgGray, imgCorrect, bboxes, imgDir, imgName, measurePtr, options, bboxPtr, yoloPtr, "");
		}
            imgRaw.release();
            imgGray.release();
            imgCorrect.release();
        }

        measurePtr.close();
	bboxPtr.close();
	yoloPtr.close();
    }
    return 0;
}
