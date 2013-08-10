// open image
// make image less detailed by downsizing
// square is scale invariant so it survives
// find lines: edge detection, meet threshold
// amplify wanted features, dim unwanted features: hereustics
// put into contour, create polygon based on contour
// conditions for square: sides, angles, size, convex

// The "Square Detector" program.
// It loads several images sequentially and tries to find squares in
// each image

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <math.h>
#include <string.h>

using namespace cv;
using namespace std;

static void help() {
    cout <<
    "\nA program using pyramid scaling, Canny, contours, contour simpification and\n"
    "memory storage (it's got it all folks) to find\n"
    "squares in a list of images pic1-6.png\n"
    "Returns sequence of squares detected on the image.\n"
    "the sequence is stored in the specified memory storage\n"
    "Call:\n"
    "./squares\n"
    "Using OpenCV version %s\n" << CV_VERSION << "\n" << endl;
}


int thresh = 50, N = 11;
const char* wndname = "Square Detection Demo";

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
static double angle( Point pt1, Point pt2, Point pt0 )
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

static int isEquilateral(vector<Point>& approx) {
	/*for (int i = 0; i < approx.size(); i++) { //print paired coordinates and individual coordinates
		cout << i << approx[i] << endl;
		cout << "x: " << approx[i].x << endl;
		cout << "y: " << approx[i].y << endl;
	}*/
	double distance[4];
	for (int i = 0; i < approx.size(); i++) {
		double dx, dy;
		dx = (approx[i].x - approx[(i + 1) % 4].x);
		dy = (approx[i].y - approx[(i + 1) % 4].y);
		distance[i] = sqrt((dx * dx) + (dy * dy));
		/*cout << "Point: " << i << approx[i] << endl;
		cout << "Distance from " << i << " to " << (i + 1) % 4 << endl
				<< distance[i] << endl;*/
	}
	for (int i = 0; i < approx.size(); i++) {
		double r = distance[i]/distance[(i+1)%4];
		if(r > 1.05 || r < 0.95)
			return 0;
	}
	return 1;
}

// returns sequence of squares detected on the image.
// the sequence is stored in the specified memory storage
static void findSquares( const Mat& image, vector<vector<Point> >& squares )
{
    squares.clear();

    Mat pyr, timg, gray0(image.size(), CV_8U), gray;
	/* Can also be written as:
    Mat pyr, timg, gray0(image.size().height, image.size().width, CV_8U), gray;
	Mat can be constructed with different arguments
	*/


    // down-scale and upscale the image to filter out the noise
    pyrDown(image, pyr, Size(image.cols/2, image.rows/2));
    pyrUp(pyr, timg, image.size());
    vector<vector<Point> > contours;

    // find squares in every color plane of the image
    for( int c = 0; c < 3; c++ )
    {
        int ch[] = {c, 0};
        mixChannels(&timg, 1, &gray0, 1, ch, 1);

        // try several threshold levels
        for( int l = 0; l < N; l++ )
        {
            // hack: use Canny instead of zero threshold level.
            // Canny helps to catch squares with gradient shading
            if( l == 0 )
            {
                // apply Canny. Take the upper threshold from slider
                // and set the lower to 0 (which forces edges merging)
                Canny(gray0, gray, 0, thresh, 5);
                // dilate canny output to remove potential
                // holes between edge segments
                dilate(gray, gray, Mat(), Point(-1,-1));
            }
            else
            {
                // apply threshold if l!=0:
                //     tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0
                gray = gray0 >= (l+1)*255/N;
            }

            // find contours and store them all as a list
            findContours(gray, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

            vector<Point> approx;

            // test each contour
            for( size_t i = 0; i < contours.size(); i++ )
            {
                // approximate contour with accuracy proportional
                // to the contour perimeter
                approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);

                // square contours should have 4 vertices after approximation
                // relatively large area (to filter out noisy contours)
                // and be convex.
                // Note: absolute value of an area is used because
                // area may be positive or negative - in accordance with the
                // contour orientation
				//
				// 3 conditions to be a square
                if( approx.size() == 4 && //4 vertices
                    fabs(contourArea(Mat(approx))) > 1000 && //greater than this size
                    isContourConvex(Mat(approx)) ) //convex
                {
                    double maxCosine = 0;

                    for( int j = 2; j < 5; j++ )
                    {
                        // find the maximum cosine of the angle between joint edges
                        double cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
                        maxCosine = MAX(maxCosine, cosine);
                    }

                    // if cosines of all angles are small
                    // (all angles are ~90 degree) then write quadrangle
                    // vertices to resultant sequence
                    if( maxCosine < 0.3 )
                        squares.push_back(approx);
                }
            }
        }
    }
}


// the function draws all the squares in the image
static void drawSquares( Mat& image, const vector<vector<Point> >& squares )
{
    for( size_t i = 0; i < squares.size(); i++ )
    {
        const Point* p = &squares[i][0];
        int n = (int)squares[i].size();
        polylines(image, &p, &n, 1, true, Scalar(0,255,0), 3, CV_AA);
    }

	Mat flipped;
	flip(image,flipped,1);
    imshow(wndname, flipped);
}


int main(int argc, char** argv)
{
    //help();
	VideoCapture capture;
	double rate;
	if (argc < 2) {
		capture.open(0); // Use default webcam if no arguments are provided
		// Get the frame rate
 		rate = 30; /* I hardcoded the frame rate for now
  	  	  	  	  	  because my webcam isn't reporting the
  	  	  	  	  	  frame rate correctly. */
	}
	else if (argc >1){
		capture.open(argv[1]); // Use video file if specified
		rate = capture.get(CV_CAP_PROP_FPS); // Use frame rate from video file
	}
	if (!capture.isOpened())
		return 1;
	std::cout << rate << endl;
	bool stop(false);

    Mat image;

    if( image.empty() ) {
       	cout << "Couldn't load " << endl;
    }
	int delay = 1000/rate;

    //namedWindow( wndname, 1 );
    vector<vector<Point> > squares;

    while(!stop)
    {
		if(!capture.read(image))
			break;
		capture >> image;
        findSquares(image, squares);
        drawSquares(image, squares);

        //int c = waitKey();
		if (cv::waitKey(delay)>=0)
			stop = true;
        /*if( (char)c == 27 )
            break;*/
    }

	capture.release();
    return 0;
}
