#include<cv.h>
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/objdetect/objdetect.hpp"
 #include "opencv2/highgui/highgui.hpp"
 #include "opencv2/imgproc/imgproc.hpp"
 #include "opencv2/video/video.hpp"
 #include "opencv2/video/tracking.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

 #include <iostream>
 #include <stdio.h>

 using namespace std;
 using namespace cv;

 /** Function Headers */
Mat detectForeground( Mat frame , Mat& Mask , BackgroundSubtractorMOG2& bgModel);
Mat detectarColor(Mat frame);
void detectAndTrackAndDisplay( Mat frame );//, vector<Point2f>& keyPoints);

 /** Global variables */
 string window_name = "Capture - Face detection";
 RNG rng(12345);
 Mat faceROI;
 const int MAX_COUNT = 500;
 const int MAX_ITER = 10;
 int morph_size=5;
 bool trackObject=false;
 int bins = 16;
 int Lmin = 30;
 int Lmax = 200;
 cv::MatND hist(2, bins, CV_8UC2);
 vector<KeyPoint> keypointsFace,keypointsFrame;
 Mat descriptorsFace, descriptorsFrame;



 /** @function main */
 int main( int argc, const char** argv )
 {
   VideoWriter output_video;
   //CvCapture* capture;
   Mat frame, Mask, mascara, original, frame_prueba;

   //-- 2. Read the video stream

   //For input video and/or default camera
   std::string arg = argv[1];

   VideoCapture capture(arg); //try to open string, this will attempt to open it as a video file

    if (!capture.isOpened()) //if this fails, try to open as a video camera, through the use of an integer param
       capture.open(atoi(arg.c_str()));
   if (!capture.isOpened()) {
       cerr << "Failed to open a video device or video file!\n" << endl;
       return 1;
   }


   //Capture screen
   /*VideoCapture capture(0);
   capture.open(0);
   capture.set(CV_CAP_PROP_FOURCC, CV_FOURCC('F', 'L', 'V', '1'));
   if (!capture.isOpened()) {
       cerr << "Failed to open a video device or video file!\n" << endl;
       return 1;
   }
   VideoWriter outStream("http://localhost:8090/feed2.ffm",
   CV_FOURCC('F', 'L', 'V', '1'), 10, Size(200, 200), true);
*/
   //For camera
   /*VideoCapture capture(1);
   if (!capture.isOpened()) {
       cerr << "Failed to open a video device or video file!\n" << endl;
       return 1;
   }
*/
   double fps = capture.get(CV_CAP_PROP_FPS); //get the frames per seconds of the video
   cout << "Frame per seconds : " << fps << endl;
   BackgroundSubtractorMOG2 bgModel;
   bgModel = BackgroundSubtractorMOG2(); // construct the class for background subtraction

   int n = 0;
   char filename[200];

   for (;;) {
       capture >> frame;
       //-- 3. Apply the classifier to the frame
       if( !frame.empty() )
       {


           mascara=detectForeground( frame , Mask , bgModel);

           //Reduce the size
           resize(frame, original, Size(), 0.5, 0.5, INTER_LINEAR);

           imshow("Video: SPACE to save a frame, q or esc to quit", original);
            output_video.write(mascara);

           //detectAndTrackAndDisplay( frame );//, &keyPoints);
       }
       else
       { printf(" --(!) No captured frame -- Break!"); break; }

       //imshow(window_name, frame);
       char key = (char)waitKey(5); //delay N millis, usually long enough to display and capture input
       switch (key) {
       case 'q':
       case 'Q':
       case 27: //escape key
           return 0;
       case 'p':
           waitKey();
           break;
       case ' ': //Save an image
           sprintf(filename,"filename%.3d.jpg",n++);
           imwrite(filename,mascara);
           cout << "Saved " << filename << endl;
           break;
       default:
           break;
       }
   }
   return 0;
 }
/** @function detectForeground */
 Mat detectForeground(Mat frame , Mat& Mask, BackgroundSubtractorMOG2& bgModel)
 {
     Mat mascara;
     std::vector<std::vector<cv::Point> > contours;
     bgModel(frame , Mask , -1);

     // get rid of noise
     erode(Mask,Mask,cv::Mat());
     dilate(Mask,Mask,cv::Mat());
     // fill holes
     Mat element = getStructuringElement( MORPH_ELLIPSE,
                                          Size( 2*morph_size + 1, 2*morph_size+1 ),
                                          Point( morph_size, morph_size ) );
     dilate(Mask,Mask,element);
     erode(Mask,Mask,element);

     //Reduce the size
     resize(Mask, mascara, Size(), 0.5, 0.5, INTER_LINEAR);

     //-- Show what you got
     string window_name = "Foreground mask: SPACE to save a frame, q or esc to quit";
     imshow( window_name, mascara==255 );// get rid of shadows
     //Apply the mask to show the image
     Mat frameAux;
     frame.copyTo(frameAux,Mask);

     //Reduce the size
     //resize(frameAux, frameAux, Size(), 0.5, 0.5, INTER_LINEAR);

     imshow("foreground",frameAux);

     return frameAux;
 }

