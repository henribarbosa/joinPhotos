#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "tclap/CmdLine.h"
#include "tclap/ValueArg.h"

struct brightnessRow
{
	int numberHeights = 5; // number of heights to measure brightness
	int frameHeight; // number of pixels in the height 
	int diameter = 1; // tube diameter
	int frame = 1; // frame to read
	std::vector<double> heights; // heights to measure the brightness 
	std::vector<int> pixelsHeight; // pixels to probe
	double* brightness; // vector of brightness

	void calculateHeights()
	{
		heights.clear();
		double dHeight = (frameHeight - 20) * (1.0 / (numberHeights - 1.0)); 
		for (int i=0; i<numberHeights; i++) {
			pixelsHeight.push_back(int(frameHeight - 10-i*dHeight));
//			std::cout << i << " : " << dHeight << "  " << (10.0+i*dHeight)<< std::endl;
			heights.push_back((frameHeight - 10.0-i*dHeight)/diameter);
		}
	}

	void makeFile()
	{
		std::ofstream file;
		file.open("Files/brightness_help.txt", std::ios::trunc);
		file << "Heights/D :";
		for (double alt : heights) {
//			std::cout << alt << std::endl;
			file << " " << alt;
		}
		file << std::endl;
		file.close();
	}

	void printFrame()
	{
		std::ofstream file;
		file.open("Files/brightness.txt", std::ios::app);
		file << frame << " :";
		for ( int i = 0; i < numberHeights; i++) 
		{
			file << " " << brightness[i];
		}
		file << std::endl;
		file.close();
		frame++;
	}
};

// read configuration files
void readFile(int& frameRate, int& width, int& interval)
{
	std::string line; // space for read the line
	std::fstream file;
	file.open("Files/characteristics.txt");
	file >> line >> frameRate;
	file >> line >> width;
	file >> line >> interval;
	file.close();
}

// threshold image to find plugs
cv::Mat processImage(cv::Mat image)
{
	cv::Mat result;
	double maxValue; // used to rescale the image
				  
	cv::minMaxIdx(image, NULL, &maxValue, NULL, NULL); // find max value
//	cv::divide(1.0/maxValue, image, image); // scale image 0 -> 1
	cv::GaussianBlur(image, result, cv::Size(51,51), 0); //blur the particles
//	cv::threshold(result, result, 0.35 * maxValue, 1.0 * maxValue, 
//			cv::THRESH_BINARY); // identify plugs

	return result;
}

void iterateFrames(cv::Mat* image, int width, brightnessRow& levels)
{
	cv::Rect window((int)(width/2), 0, 1, image->rows); // window to look 
														// for plugs
	int count = 0;
	while (window.x < image->cols) 
	{
		count++; //std::cout << "time: " << count << std::endl;
		cv::Mat column = (*image)(window); // column examined
		levels.brightness = new double[levels.numberHeights];
										   
		for (int j = 0; j < levels.numberHeights; j++)
		{
			levels.brightness[j] = column.at<uchar>(levels.pixelsHeight[j],0);
		}

		levels.printFrame();
		window = window + cv::Point(width,0); // moves the window to next frame
											  
		// free memory
		delete[] levels.brightness;
	}

}

int main()
{
	// read quantities
	int frameRate, width, interval;
	readFile(frameRate, width, interval);

	// setup struct to save data
	brightnessRow levels;
	levels.numberHeights = 51;
	levels.diameter = width;

	// open joined image
	cv::Mat image = cv::imread("joined.png", cv::IMREAD_GRAYSCALE);
	cv::Mat exibit;
	cv::cvtColor(image, exibit, cv::COLOR_GRAY2RGB);
	levels.frameHeight = image.rows;
	levels.calculateHeights();
	levels.makeFile();

	cv::Mat processedImage;
	processedImage = processImage(image);

	iterateFrames(&processedImage, width, levels);

//	for ( int i = 0; i < levels.numberHeights; i++)
//	{
//		cv::line(exibit, cv::Point(0,levels.pixelsHeight[i]), 
//				cv::Point(exibit.cols, levels.pixelsHeight[i]), 
//				cv::Scalar(0,0,255),3);
//	}

	// graphic window to display velocity of the particles
//	cv::namedWindow("Processed", cv::WINDOW_NORMAL);
//	cv::resizeWindow("Processed", 1200,50);
//	cv::imshow("Processed", exibit);
//	cv::setWindowProperty("Processed", cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);
//	cv::waitKey(0);


	return 0;
}

