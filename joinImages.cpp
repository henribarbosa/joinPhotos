#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

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

// main function
int main(int argc, char *argv[])
{
	// Constants
	int initFrame = std::stoi(std::string(argv[2]));
	int skipFrame = std::stoi(std::string(argv[3]));
	int endFrame  = std::stoi(std::string(argv[4]));
	int height = std::stoi(std::string(argv[5]));
	float D = 3.0; // bed diameter
	
	std::string path(argv[1]);
	//std::cout << path << std::endl;

	//cv::namedWindow("Joined Image",cv::WINDOW_NORMAL);
	
	// first frame
	std::stringstream initFrameStr;
	initFrameStr << std::setfill('0') << std::setw(5) << initFrame;
//	initFrame += skipFrame;
	cv::Mat joinedImage = imread(path + "image_" + initFrameStr.str() + ".tif", cv::IMREAD_GRAYSCALE);
	cv::rotate(joinedImage, joinedImage, cv::ROTATE_90_CLOCKWISE);

	// find the borders of the bed
	int right, left, bottom;
	firstPass(joinedImage, right, left, bottom);
	float scale = (float)(left - right) / 3.0; // bed scale
	
	// text scale
	float ImScale = (left - right) * 0.04;

//	cv::Mat labelImage(60, joinedImage.rows, CV_8U, cv::Scalar(255));
	// y-axis label
	if (makeLegend)
	{
		cv::Mat labelImage(60, height, CV_8U, cv::Scalar(255));
		cv::putText(labelImage, "H/D", cv::Point(height/2 - 25, 60), cv::FONT_HERSHEY_DUPLEX, ImScale, cv::Scalar(0), 2);
		cv::rotate(labelImage, labelImage, cv::ROTATE_90_COUNTERCLOCKWISE);
	
		std::stringstream heightlabel;
		heightlabel << std::setfill(' ') << std::setw(3) << 0;

		// y-axis values
		cv::Mat scaleImage(height, 200, CV_8U, cv::Scalar(255));
		cv::Point origin(60,height);
		cv::putText(scaleImage, heightlabel.str().c_str(), origin, cv::FONT_HERSHEY_DUPLEX, ImScale, cv::Scalar(0), 2);
	
		int step = 5;
		int heightTxt = step;
		while (true)
		{
			heightlabel.str(std::string());
			heightlabel << std::setfill(' ') << std::setw(3) << heightTxt;
			origin -= cv::Point(0,(int)(step*scale*D));
	
			if (origin.y <= 0)
				break;

			cv::putText(scaleImage, heightlabel.str().c_str(), origin, cv::FONT_HERSHEY_DUPLEX, ImScale, cv::Scalar(0), 2);
			heightTxt += step;
		}


		//cv::namedWindow("Scale", cv::WINDOW_NORMAL);
		//cv::imshow("Scale", scaleImage);
		//cv::waitKey(0);

	//	cv::Rect frame(right,0,left-right,joinedImage.rows);
		cv::Rect frame(right,joinedImage.rows-height,left-right,height);
		cv::hconcat(labelImage, scaleImage, joinedImage);
	}
	else
	{
		cv::Rect frame(right,bottom-height,left-right,height);
		joinedImage = joinedImage(frame);
	}

	// concatenate all frames
	if (not makeLegend) { initFrame += skipFrame; }
	for (int frames = initFrame; frames <= endFrame; frames = frames + skipFrame)
	{
		std::stringstream frameNumber;
		frameNumber << std::setfill('0') << std::setw(5) << frames;

		cv::Mat frameImage = cv::imread(path + "image_" + frameNumber.str() + ".tif", cv::IMREAD_GRAYSCALE);
		cv::rotate(frameImage, frameImage, cv::ROTATE_90_CLOCKWISE);

//		int right, left; 
//		firstPass(frameImage, right, left);
//		cv::Rect framerect(right,0,left-right,frameImage.rows);
		cv::Rect framerect(right,bottom-height,left-right,height);
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
		float step = endTime / 5.0;
//		std::cout << endTime << " : " << step << std::endl;
		float heightTxt = step;
		while (true)
		{
			timelabel.str(std::string());
			timelabel << std::setfill(' ') << std::setw(3) << heightTxt;
			xorigin += cv::Point((int)(step*frameRate/skipFrame*(left - right)),0);
//			std::cout << step*frameRate/skipFrame*(left-right) << " , " << xorigin.x << std::endl;

			if (xorigin.x >= joinedImage.cols)
				break;

			cv::putText(timeLabelImage, timelabel.str().c_str(), xorigin, cv::FONT_HERSHEY_DUPLEX, ImScale, cv::Scalar(0), 2);
			heightTxt += step;
		}
		cv::vconcat(joinedImage, timeLabelImage, joinedImage);
	}
	//cv::imshow("JoinedImages", joinedImage);
	cv::imwrite("joined.png", joinedImage);

	return 0;
}

