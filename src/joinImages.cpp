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

int frameRate = 1000; //adjust frame rate
bool makeLegend = true; //prints legends

// edge detection algorithm to detect the borders of the bed
void GaussianDifference(cv::Mat& input, cv::Mat& output, int sigma, double k)
// difference of gaussian blurs with sigma and k*sigma
{
	cv::Mat g1, g2;
	cv::GaussianBlur(input, g1, cv::Size(0, 0), sigma, sigma);
	cv::GaussianBlur(input, g2, cv::Size(0, 0), k * sigma, k * sigma);
	cv::subtract(g1, g2, output);
	output *= 4;
}

// find the borders of the first image
void firstPass(cv::Mat& image, int& right, int& left, int& bottom)
//set lines for right and left limits of the bed
{
//	cv::namedWindow("Lines", cv::WINDOW_NORMAL);
//	cv::resizeWindow("Lines", 1200, 50);

	cv::Mat imageProcess;

	// edge detection
	GaussianDifference(image, imageProcess, 1, 3);

	// threshold to find borders
	int minimumSum = 10000;

	// vertical searching
	for (int i = 0; i < image.cols ; i++)
	{
		cv::Rect slide(i, 0, 1, image.rows);

		cv::Scalar testSum = cv::sum(imageProcess(slide));
		if (testSum[0] > minimumSum)
		{
			right = i; // right border
			break;
		}
	}

	for (int i = image.cols-1; i > 0; i--)
	{
		cv::Rect slide(i, 0, 1, image.rows);

		cv::Scalar testSum = cv::sum(imageProcess(slide));
		if (testSum[0] > minimumSum)
		{
			left = i; // left border
			break;
		}
	}

	for (int i = image.rows-1; i > 0; i--) 
	{
		cv::Rect slide(0, i, image.cols, 1);

		cv::Scalar testSum = cv::sum(imageProcess(slide));
		if (testSum[0] > minimumSum/10)
		{
			bottom = i;//image.rows-1; bottom of the bed
//			std::cout << bottom << std::endl;
			break;
		}
	}

	//std::cout << right << " , " << left << " , " << bottom << std::endl;

//	cv::line(show, cv::Point(right, 0), cv::Point(right, show.rows), cv::Scalar(255, 0, 0),2,8);
//	cv::line(show, cv::Point(left, 0), cv::Point(left, show.rows), cv::Scalar(255, 0, 0), 2, 8);

//	cv::imshow("Lines", show);
//	cv::waitKey(500);
//	cv::destroyWindow("Lines");
}

// join function
int join(std::string ImagesPath, int initFrame, int skipFrame, int endFrame, int imageHeight, int frameRate, bool makeLegend, int yscale, int numberXdivisions, float textScale)
{
	// first frame
	std::stringstream initFrameStr;
	initFrameStr << std::setfill('0') << std::setw(5) << initFrame;
//	initFrame += skipFrame;
	cv::Mat joinedImage = imread(ImagesPath + "image_" + initFrameStr.str() + ".tif", cv::IMREAD_GRAYSCALE);
	cv::rotate(joinedImage, joinedImage, cv::ROTATE_90_CLOCKWISE);

	// find the borders of the bed
	int right, left, bottom;
	firstPass(joinedImage, right, left, bottom);
	float scale = (float)(left - right); // bed scale
	
	// text scale
	float ImScale = (left - right) * textScale;

//	cv::Mat labelImage(60, joinedImage.rows, CV_8U, cv::Scalar(255));
	// y-axis label
	if (makeLegend)
	{
		cv::Mat labelImage(60, imageHeight, CV_8U, cv::Scalar(255));
		cv::putText(labelImage, "H/D", cv::Point(imageHeight/2 - 25, 60), cv::FONT_HERSHEY_DUPLEX, ImScale, cv::Scalar(0), 2);
		cv::rotate(labelImage, labelImage, cv::ROTATE_90_COUNTERCLOCKWISE);
	
		int yInit = 5; // the bed image start after 5 diameters
		std::stringstream heightlabel;
		heightlabel << std::setfill(' ') << std::setw(3) << yInit;

		// y-axis values
		cv::Mat scaleImage(imageHeight, 200, CV_8U, cv::Scalar(255));
		cv::Point origin(60,imageHeight);
		cv::putText(scaleImage, heightlabel.str().c_str(), origin, cv::FONT_HERSHEY_DUPLEX, ImScale, cv::Scalar(0), 2);
	
		int heightTxt = yscale + yInit;
		while (true)
		{
			heightlabel.str(std::string());
			heightlabel << std::setfill(' ') << std::setw(3) << heightTxt;
			origin -= cv::Point(0,(int)(yscale * scale));
	
			if (origin.y <= 0)
				break;

			cv::putText(scaleImage, heightlabel.str().c_str(), origin, cv::FONT_HERSHEY_DUPLEX, ImScale, cv::Scalar(0), 2);
			heightTxt += yscale;
		}


		//cv::namedWindow("Scale", cv::WINDOW_NORMAL);
		//cv::imshow("Scale", scaleImage);
		//cv::waitKey(0);

	//	cv::Rect frame(right,0,left-right,joinedImage.rows);
//		cv::Rect frame(right,joinedImage.rows-imageHeight,left-right,imageHeight);
		cv::hconcat(labelImage, scaleImage, joinedImage);
	}
	else
	{
		cv::Rect frame(right,bottom-imageHeight,left-right,imageHeight);
		joinedImage = joinedImage(frame);
	}

	// concatenate all frames
	if (not makeLegend) { initFrame += skipFrame; }
	for (int frames = initFrame; frames <= endFrame; frames = frames + skipFrame)
	{
		std::stringstream frameNumber;
		frameNumber << std::setfill('0') << std::setw(5) << frames;

		cv::Mat frameImage = cv::imread(ImagesPath + "image_" + frameNumber.str() + ".tif", cv::IMREAD_GRAYSCALE);
		cv::rotate(frameImage, frameImage, cv::ROTATE_90_CLOCKWISE);

//		int right, left; 
//		firstPass(frameImage, right, left);
//		cv::Rect framerect(right,0,left-right,frameImage.rows);
		cv::Rect framerect(right,bottom-imageHeight,left-right,imageHeight);
		frameImage = frameImage(framerect);
		
		cv::hconcat(joinedImage, frameImage, joinedImage);

		//cv::imshow("Joined Image", joinedImage);
		//cv::waitKey(1000);
	}

	if (makeLegend)
	{
		// puts the time stamps
		cv::Mat timeLabelImage(180, joinedImage.cols, CV_8U, cv::Scalar(255));
		cv::putText(timeLabelImage, "t [s]", cv::Point(joinedImage.cols/2 + 25, 150), cv::FONT_HERSHEY_DUPLEX, ImScale, cv::Scalar(0), 2);

		std::stringstream timelabel;
		timelabel << std::setfill(' ') << std::setw(3) << 0;

		// x-axis values
		cv::Point xorigin(200,60);
		cv::putText(timeLabelImage, timelabel.str().c_str(), xorigin, cv::FONT_HERSHEY_DUPLEX, ImScale, cv::Scalar(0), 2);
	
		float endTime = (endFrame - initFrame) / (frameRate);
		float step = endTime / (numberXdivisions * 1.0);
//		std::cout << endTime << " : " << step << std::endl;
		float heightTxt = step;
		while (true)
		{
			timelabel.str(std::string());
			timelabel << std::setfill(' ') << std::setw(3) << heightTxt;
			xorigin += cv::Point((int)(step * frameRate / skipFrame * (left - right)),0);
//			std::cout << step*frameRate/skipFrame*(left-right) << " , " << xorigin.x << std::endl;

			if (xorigin.x >= joinedImage.cols)
				break;

			cv::putText(timeLabelImage, timelabel.str().c_str(), xorigin, cv::FONT_HERSHEY_DUPLEX, ImScale, cv::Scalar(0), 2);
			heightTxt += step;
		}
		cv::vconcat(joinedImage, timeLabelImage, joinedImage);
	}
	else // write file for plug analysis
	{
		std::ofstream file;
		file.open("Files/characteristics.txt",std::ios::trunc);
		file << "fps " << frameRate << std::endl;
		file << "width " << left - right << std::endl;
		file << "interval " << skipFrame << std::endl;
		file.close();
	}
	//cv::imshow("JoinedImages", joinedImage);
	cv::imwrite("joined.png", joinedImage);

	return 0;
}

// main function
int main(int argc, char** argv)
{
	// Define variables
	int frameRate = 1000;
	int imageHeight = 1000;
	int initFrame, skipFrame, endFrame; // first frame, frames to skip ans last frame
	bool makeLegend = true;
	int yscale = 5, numberXdivisions = 5; // H/D smallest division and number of times in x axis
	float textScale = 0.04; // text scale for labels
	std::string ImagesPath; // path to the frames

	try
	{
		// Build command line arguments for this program
		TCLAP::CmdLine cmd("Program to join frames", ' ', "1.0");

		// Requires a path to access the frames
		TCLAP::ValueArg<std::string> FramesPathTC("p", "FramesPath", "[P]ath to the frames to be joined", true, "", "string");
		cmd.add(FramesPathTC);

		// Firt frame to be joined
		TCLAP::ValueArg<int> FirstFrameTC("i", "FirstFrame", "[I]nitial frame to be joined", true, 0, "int");
		cmd.add(FirstFrameTC);

		// Last frame to be joined
		TCLAP::ValueArg<int> LastFrameTC("l", "EndFrame", "[L]ast frame to be joined", true, 1000, "int");
		cmd.add(LastFrameTC);

		// Interval of frames to skip
		TCLAP::ValueArg<int> SkipFrameTC("d", "SkipFrames", "[D]elta of frames between consecutive images", true, 50, "int");
		cmd.add(SkipFrameTC);

		// Height of the final image
		TCLAP::ValueArg<int> HeightTC("v", "ImageHeight", "[V]ertical size of the final image (pixels)", true, imageHeight, "int");
		cmd.add(HeightTC);

		// Frame rate
		TCLAP::ValueArg<int> frameRateTC("f", "frameRate", "[F]rame rate of images (default 1000)", false, frameRate, "int");
		cmd.add(frameRateTC);

		// Prints axis labels
		TCLAP::ValueArg<bool> makeLabelsTC("a", "makeAxis", "Prints the [A]xis (default true)", false, makeLegend, "bool");
		cmd.add(makeLabelsTC);

		// Height smallest division
		TCLAP::ValueArg<int> yscaleTC("y", "yScale", "Smallest division of [Y]-axis (default H/D = 5)", false, yscale, "int");
		cmd.add(yscaleTC);

		// Number of time divisions
		TCLAP::ValueArg<int>numberTimeLabelsTC("x", "xDivisions", "Number of times in the [X]-axis (default 5)", false, numberXdivisions, "int");
		cmd.add(numberTimeLabelsTC);

		// Text scale for labels
		TCLAP::ValueArg<float> textScaleTC("s", "textScale", "Text [S]cale for the labels (default 0.04)", false, textScale, "float");
		cmd.add(textScaleTC);

		// reads the command line arguments
		cmd.parse(argc, argv);

		// Atribute the variables
		ImagesPath = FramesPathTC.getValue();
		initFrame = FirstFrameTC.getValue();
		skipFrame = SkipFrameTC.getValue();
		endFrame = LastFrameTC.getValue();
		imageHeight = HeightTC.getValue();
		frameRate = frameRateTC.getValue();
		makeLegend = makeLabelsTC.getValue();
		yscale = yscaleTC.getValue();
		numberXdivisions = numberTimeLabelsTC.getValue();
		textScale = textScaleTC.getValue();
	}
	catch (TCLAP::ArgException &e)
	{
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
	}

	int result = join(ImagesPath, initFrame, skipFrame, endFrame, imageHeight, frameRate, makeLegend, yscale, numberXdivisions, textScale);

	return result;
}
		
