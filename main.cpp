<<<<<<< HEAD
#include <cv.h>
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/video/tracking.hpp"
//#include "opencv2/nonfree/nonfree.hpp"

#include <iostream>
#include <stdio.h>

 using namespace std;
 using namespace cv;

int main( int argc, char** argv )
{

       //Para vídeo o cámara por defecto
       std::string arg = argv[1];

   VideoCapture cap(arg); //capture the video from web cam

   // Si falla, activa la cámara por defecto
    if (!cap.isOpened())
        cap.open(atoi(arg.c_str()));
    if (!cap.isOpened()) {
        cerr << "No se ha podido abrir el archivo ni activar la cámara\n" << endl;
        return 1;
    }

   namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"

 int iLowH = 0;
int iHighH = 255;

 int iLowS = 0;
int iHighS = 255;

 int iLowV = 0;
int iHighV = 255;

 //Create trackbars in "Control" window
cvCreateTrackbar("LowH", "Control", &iLowH, 255); //Hue (0 - 179)
cvCreateTrackbar("HighH", "Control", &iHighH, 255);

 cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
cvCreateTrackbar("HighS", "Control", &iHighS, 255);

 cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
cvCreateTrackbar("HighV", "Control", &iHighV, 255);

   while (true)
   {
       Mat imgOriginal;

       bool bSuccess = cap.read(imgOriginal); // read a new frame from video

        if (!bSuccess) //if not success, break loop
       {
            cout << "Cannot read a frame from video stream" << endl;
            break;
       }

   Mat imgHSV;

  cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

 Mat imgThresholded;

  inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

 //morphological opening (remove small objects from the foreground)
 erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
 dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

  //morphological closing (fill small holes in the foreground)
 dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
 erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

  imshow("Thresholded Image", imgThresholded); //show the thresholded image
 imshow("Original", imgOriginal); //show the original image
 waitKey();

       if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
      {
           cout << "esc key is pressed by user" << endl;
           break;
      }

   }

  return 0;

}













// /** Funciones */
//Mat detectForeground( Mat frame , Mat& Mask , BackgroundSubtractorMOG2& bgModel);
//Mat detecta_contorno(Mat img, Mat frame);
//Vector<Mat> detecta_color(vector<Rect> rect, Mat frame);


///** Variables globales */
//string window_name = "Capture - Face detection";
//static int Xmin=0, Ymin=0, Xmax=0, Ymax=0; //Encuadrar
//const int MAX_COUNT = 500;
//const int MAX_ITER = 10;
//int morph_size=5;
//bool trackObject=false;
//int bins = 16;
//int Lmin = 30;
//int Lmax = 200;
//cv::MatND hist(2, bins, CV_8UC2);
//vector<KeyPoint> keypointsFace,keypointsFrame;
//Mat descriptorsFace, descriptorsFrame;

// /** @function main */
// int main( int argc, const char** argv )
// {
//   VideoWriter output_video;
//   Mat frame, Mask, mascara, original, cuadra;

//   //Para vídeo o cámara por defecto
//   std::string arg = argv[1];

//   // Abre el archivo de entrada
//   VideoCapture capture(arg);

//   // Si falla, activa la cámara por defecto
//    if (!capture.isOpened())
//        capture.open(atoi(arg.c_str()));
//    if (!capture.isOpened()) {
//        cerr << "No se ha podido abrir el archivo ni activar la cámara\n" << endl;
//        return 1;
//    }

//   //Cámara secundaria
//   /*VideoCapture capture(1);
//   if (!capture.isOpened()) {
//       cerr << "Failed to open a video device or video file!\n" << endl;
//       return 1;
//   }
//*/
//   // Obtiene los FPS
//   double fps = capture.get(CV_CAP_PROP_FPS);
//   cout << "Frames per second: " << fps << endl;
//   BackgroundSubtractorMOG2 bgModel;

//   // Construye la clase para background substraction
//   bgModel = BackgroundSubtractorMOG2();

//   // Usado para guardar imágenes
//   int n = 0;
//   char filename[200];
//   Mat img;

//   for (;;) {
//       capture >> frame;
//       if( !frame.empty() )
//       {
//           mascara=detectForeground( frame , Mask , bgModel);
//           //Reduce el tamaño de ventana
//           resize(frame, original, Size(), 0.5, 0.5, INTER_LINEAR);
//       }
//       else
//       { printf(" --(!) No captured frame -- Break!"); break; }

//       // Retarda N millis, suficiente para mostrar y capturar comandos
//       char key = (char)waitKey(5);
//       switch (key) {
//       case 'q':
//       case 'Q':
//       case 27: //escape key
//            return 0;
//       case ' ': //Guarda una imagen
//            sprintf(filename,"filename%.3d.jpg",n++);
//            imwrite(filename,frame);
//            cout << "Guardada " << filename << endl;
//            break;
//       case 'p': //Pausa
//           // img = detecta_contorno(mascara==255,frame);
//            waitKey();
//            break;
//       default:
//            break;
//       }
//   }
//   return 0;
// }

///** @function detectForeground
//    Enmascara la imagen*/
// Mat detectForeground(Mat frame , Mat& Mask, BackgroundSubtractorMOG2& bgModel)
// {
//     Mat mascara;
//     std::vector<std::vector<cv::Point> > contours;
//     bgModel(frame , Mask , -1);

//     // Eliminamos ruido
//     erode(Mask,Mask,cv::Mat());
//     dilate(Mask,Mask,cv::Mat());

//     // Rellena huecos
//     Mat element = getStructuringElement( MORPH_ELLIPSE,
//                                          Size( 4*morph_size + 1, 4*morph_size+1 ),
//                                          Point( morph_size, morph_size ) );
//     dilate(Mask,Mask,element);
//    // erode(Mask,Mask,element);

//     //Reduce tamaño de ventana
//     resize(Mask, mascara, Size(), 0.5, 0.5, INTER_LINEAR);
//     string window_name = "Foreground mask: SPACE to save a frame, q or esc to quit";
//     imshow( window_name, mascara==255);// get rid of shadows

//     //Aplica la máscara sobre la imagen
//     Mat frameAux;
//     frame.copyTo(frameAux,Mask);

//     //Reduce tamaño de ventana
//     resize(frameAux, frameAux, Size(), 0.5, 0.5, INTER_LINEAR);
//     imshow("foreground",frameAux);

//     //Obtenemos los objetos de la máscara
//     Mat img=detecta_contorno(Mask==255, frame);

//     return Mask;
// }

// /** @function detecta_contorno
//  * Busca los objetos a partir
//  * de los bordes que encuentre
//  * en la máscara, y los encuadra **/

// Mat detecta_contorno(Mat img, Mat frame){
//     vector<vector<Point> > contours;
//     findContours(img, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
//     // Dibuja los objetos en blanco
//     drawContours(img, contours,-1,0,-1,8,noArray(),255,Point());

//     // Encuadramos los objetos obtenidos
//     vector<vector<Point> > contours_poly(contours.size());
//     vector<Rect> boundRect(contours.size());
//     for(int i=0; i<contours.size(); i++){
//         approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
//         boundRect[i] = boundingRect(Mat(contours_poly[i]));
//     //    rectangle(frame, boundRect[i], 100, 2, 8, 0);
//     }
//     detecta_color(boundRect,frame);
//     imshow("Contornos", frame);
//     return img;
// }

// Vector<Mat> detecta_color(vector<Rect> rect, Mat frame){
//    Mat rectSection;
//    Mat rectSectionHSV;
//    Mat rectInterested;
//    vector<vector<Point> > contours;
//    Vector<Mat> rectInterestedVector;
//    for (int x=0; x < rect.size(); x++){
//     rectSection = frame(rect[x]);
//     cvtColor(rectSection,rectSectionHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
//    // inRange(rectSectionHSV, Scalar(0,0,0), Scalar(255, 255, 0),rectSectionHSV); //Threshold the image
//     inRange(rectSectionHSV, Scalar(0,125, 127), Scalar(255, 255, 255),rectInterested); //Threshold the image
//    // inRange(rectSectionHSV, Scalar(0,155, 0), Scalar(67, 255, 93),rectInterested); //Threshold the image

//     //morphological opening (remove small objects from the foreground)
//     erode(rectInterested, rectInterested, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
//     dilate( rectInterested, rectInterested, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

//      //morphological closing (fill small holes in the foreground)
//     dilate( rectInterested, rectInterested, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
//     erode(rectInterested, rectInterested, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

//     findContours(rectInterested, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
//     if(contours.size() != 0){
//        rectInterestedVector.push_back(rectInterested);
//        rectangle(frame, rect[x], 100, 2, 8, 0);
//     }
// }
//    return rectInterestedVector;

// }




=======
>>>>>>> origin/master


