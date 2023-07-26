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

struct plug
{
	int top, bottom; // plug measures
	int label = 0; // plug label to identification
	int frames = 0; //number of frames following this plug
	bool used = false; // was it used this frame?
	int distance = 0; // the distance used to fit this plug to the 
					  // current frame
	int nextLabel = -1; // which plug should be next
	int time = 0; // time in the movie
					
	void printTmp()
	{
		std::ofstream file;
		file.open("Files/plug_" + std::to_string(label) + ".txt", std::ios::app);
		file << time << " " << top << " " << bottom << std::endl;
		file.close();
	}

	void printFinal(cv::Mat* exibt, int width, int frameRate, int interval)
	{ 
		double dt = 1.0*interval / frameRate;

		std::ifstream readFile;
		readFile.open("Files/plug_" + std::to_string(label) + ".txt",
				std::ios::app);

		std::ofstream writeFile;
		writeFile.open("Files/plugs.txt", std::ios::app);
		writeFile << label << " ";

		std::ofstream celerityFile;
		celerityFile.open("Files/celerities.txt", std::ios::app);
		
		int pointTop, pointBottom, pointTime, pointMid;
		int prePointTop, prePointBottom, prePointMid;

		readFile >> pointTime >> prePointTop >> prePointBottom;
		writeFile << (exibt->rows - pointTop)*1.0/width << " " <<
			(exibt->rows - pointBottom)*1.0/width << " ";
		prePointMid = (int)((prePointTop + prePointBottom) / 2);

		while ( readFile >> pointTime >> pointTop >> pointBottom )
		{
			// plug positions
			writeFile << (exibt->rows - pointTop)*1.0/width << " " <<
			(exibt->rows - pointBottom)*1.0/width << " ";
			
			// middle point
			pointMid = (int)((pointTop + pointBottom) / 2);
			// visual display
			cv::line((*exibt), cv::Point((pointTime-2)*width,prePointMid), 
					cv::Point((pointTime-1)*width,pointMid),
				cv::Scalar(0,0,255), 3);

			// plug celerities based on different regions
			celerityFile << -(pointTop - pointBottom)*1.0/width << " "
				<< -(pointTop - prePointTop)*1.0/width * dt << " "
				<< -(pointBottom - prePointBottom)*1.0/width * dt << " "
				<< -(pointMid - prePointMid)*1.0/width * dt << std::endl;

//			std::cout << (pointTop - prePointTop)*1.0/width << std::endl;
//			std::cout << dt << std::endl;

			// update variables
			prePointMid = pointMid;
			prePointTop = pointTop;
			prePointBottom = pointBottom;

		}
		writeFile << std::endl;

		readFile.close();
		writeFile.close();
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
	cv::threshold(result, result, 0.35 * maxValue, 1.0 * maxValue, 
			cv::THRESH_BINARY); // identify plugs

	return result;
}

void iterateFrames(cv::Mat* image, cv::Mat* exibt, int frameRate,
		int width, int interval, std::vector<plug> lastFrame)
{
	cv::Rect window((int)(width/2), 0, 1, image->rows); // window to look 
														// for plugs
	int currentLabel = 0;
	int count = 0;
	
	while (window.x < image->cols) 
	{
		count++; std::cout << "time: " << count << std::endl;
		cv::Mat column = (*image)(window); // column examined
		bool inPlug = false; // are we inside a plug?
		std::vector<plug> currentFrame;

		for (int j = 0; j < image->rows; j++)
		{
			plug test;
			int value = column.at<uchar>(j,0);
			// beginning of the plug
			if ( (not inPlug) and (value != 0) )
			{
				inPlug = true;
				test.top = j;
				cv::circle(*exibt, cv::Point(window.x,j), 10, 
						cv::Scalar(0,255,0), -1);
			}
			// end of the plug
			else if ( inPlug and (value == 0) )
			{
				inPlug = false;
				test.bottom = j;
				test.time = count;
				cv::circle(*exibt, cv::Point(window.x,j), 10, 
						cv::Scalar(255,0,0), -1);

				int bestDistance = 10000;
				std::cout << "newplug: ";
				for (int i = 0; i < lastFrame.size(); i++)
				{
					int distance = lastFrame[i].top - test.top;
					std::cout << " d"<< distance << " ";
					// not the correct plug if it goes downward
					if ( distance < -10 )
						continue;
					distance = (distance < 0)?(-distance):distance; //absolute
					// already found a better candidate
					if ( distance > bestDistance )
					{
						std::cout << " d>" << bestDistance;
						continue;
					}
					// wants to be part of a plug unused
					if (not lastFrame[i].used)  
					{
						bestDistance = distance;
						lastFrame[i].distance = distance;
						lastFrame[i].used = true;
						test.used = true;
						test.label = lastFrame[i].label;
						test.frames = lastFrame[i].frames + 1;
						test.nextLabel = i;
						std::cout << " : " << test.label;
					}
					// wants to be part of a used plug (needs to remove
					// previous connection)
					else if (distance < lastFrame[i].distance)
					{
						bestDistance = distance;
						lastFrame[i].distance = distance;
						lastFrame[i].used = true;
						currentFrame[currentFrame.size()-1].label = 0;
						currentFrame[currentFrame.size()-1].used = false;
						currentFrame[currentFrame.size()-1].frames = 0;
						lastFrame[test.nextLabel].used = false;
						test.used = true;
						test.label = lastFrame[i].label;
						test.frames = lastFrame[i].frames + 1;
						test.nextLabel = i;
						std::cout << " : " << test.label;
					}
				}
				std::cout << "-> " << test.label << std::endl;

				currentFrame.push_back(test);
//				std::cout << currentFrame.size() << " ";
			}
			else
				continue;

		}

		for (int i = 0; i < currentFrame.size(); i++)
		{
			if ( currentFrame[i].used )
			{
				currentFrame[i].printTmp(); // store values temporaly
				currentFrame[i].used = false; // clean for next iteration
			}
			else 
			{
				currentLabel += 1;
				currentFrame[i].label = currentLabel; // creates new plug
				currentFrame[i].printTmp(); //store values temporaly
			}
		}
		for (int i = 0; i < lastFrame.size(); i++)
		{
			if ( (not lastFrame[i].used) and (lastFrame[i].frames > 5) )
			{
				std::cout << lastFrame[i].label << " " << lastFrame[i].frames << std::endl;
//				// writes definitivily a plug to the final file when it is lost
				lastFrame[i].printFinal(exibt, width, frameRate, interval);
			}
		}

		// update variables
		lastFrame.clear();
		lastFrame = currentFrame;
//		currentFrame.clear();

//		cv::namedWindow("Test", cv::WINDOW_NORMAL);
//		cv::imshow("Test", column);
//		cv::waitKey(10);

		window = window + cv::Point(width,0); // moves the window to next frame
	}
	for (int i = 0; i < lastFrame.size(); i++)
	{
			// writes the plugs at final time
		if (lastFrame[i].frames > 5)
		{
			std::cout << lastFrame[i].label << " " << lastFrame[i].frames << std::endl;
			lastFrame[i].printFinal(exibt, width, frameRate, interval);
		}
	}

}

int main()
{
	// clear files
	std::ofstream file;
	file.open("Files/plugs.txt", std::ios::trunc);
	file << "Plugs (frame top bottom top bottom ...) unit of Diameters" << std::endl;
	file.close();
	file.open("Files/celerities.txt", std::ios::trunc);
	file << "Celerities (plug_length top_celerity bottom_celerity center_celerity) unit of Diameters and Diameters/s" << std::endl;
	file.close();
	int ret = system("rm Files/plug_*.txt");

	int frameRate, width, interval;
	readFile(frameRate, width, interval);
//	std::cout << frameRate << " " << width << " " << interval << std::endl;

	// open joined image
	cv::Mat image = cv::imread("joined.png", cv::IMREAD_GRAYSCALE);
	cv::Mat exibit;
	cv::cvtColor(image, exibit, cv::COLOR_GRAY2RGB);

	cv::Mat binaryImage;
	binaryImage = processImage(image);

	std::vector<plug> lastFramePlugs; // plugs in last frame
	iterateFrames(&binaryImage, &exibit, frameRate, width, interval,
			lastFramePlugs);

	// graphic window to display velocity of the particles
	cv::namedWindow("Processed", cv::WINDOW_NORMAL);
//	cv::resizeWindow("Processed", 1200,50);
	cv::imshow("Processed", exibit);
	cv::waitKey(100);


	return 0;
}

