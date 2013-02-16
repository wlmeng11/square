#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace std;

cv::Mat function() {
	//create image
	cv::Mat ima(340/*height*/,320/*width*/,CV_8U,cv::Scalar(100));
	//return it
	return ima;
}

int main() {
	cv::Mat image;
	std::cout << "size: " << image.size().height << " , " << image.size().width << std::endl;
	image = cv::imread("shapes.png");
	if (!image.data) {
		// no image has been created...
		cerr << "No image data" << endl;
		return -1;
	}

	std::cout << "size: " << image.size().height << " , " << image.size().width << std::endl;
	//cv::namedWindow("Original Image"); //define the window
	//cv::imshow("Original Image", image); //show the image

	cv::Mat result;
	cv::flip(image,result,0); /* positive for horizontal
								 0 fo vertical
								 negative for both */
	//cv::namedWindow("Flipped Image");
	//cv::imshow("Flipped Image", result);
	//cv::imwrite("output.png", result);

	cv::Mat image2, image3;
	image2 = result;
	result.copyTo(image3);

	cv::flip(image,result,1);

	cv::imshow("Image2", image2); //image2 now matches the new flip
	cv::imshow("Image3", image3); //image3 is a copy of the old flip

	cv::Mat gray = function();
	cv::imshow("Gray", gray); //generated gray image
 

	cv::waitKey(0);
	return 0;
}
