#include<cv.h>
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/video/tracking.hpp"
#include "opencv2/nonfree/nonfree.hpp"

#include <iostream>
#include <stdio.h>

 using namespace std;
 using namespace cv;

//No se que me dice Waheed

 /** Funciones */
Mat detectForeground( Mat frame , Mat& Mask , BackgroundSubtractorMOG2& bgModel);
Mat detecta_contorno(Mat img, Mat frame);
vector<Mat> histograma(vector<Rect> boundRect, Mat frame);
Point2i angulo(vector<Rect> boundRect, int i, Mat frame);
void sliders(Mat mascara);

/** Variables globales */
string window_name = "Capture - Face detection";
static int Xmin=0, Ymin=0, Xmax=0, Ymax=0; //Encuadrar
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
   Mat img, frame, Mask, mascara, original, cuadra;

   //Para vídeo o cámara por defecto
   std::string arg = argv[1];

   // Abre el archivo de entrada
   VideoCapture capture(arg);

   // Si falla, activa la cámara por defecto
    if (!capture.isOpened())
        capture.open(atoi(arg.c_str()));
    if (!capture.isOpened()) {
        cerr << "No se ha podido abrir el archivo ni activar la cámara\n" << endl;
        return 1;
    }

   //Cámara secundaria
   /*VideoCapture capture(1);
   if (!capture.isOpened()) {
       cerr << "Failed to open a video device or video file!\n" << endl;
       return 1;
   }
*/
   // Obtiene los FPS
   double fps = capture.get(CV_CAP_PROP_FPS);
   cout << "Frames per second: " << fps << endl;
   BackgroundSubtractorMOG2 bgModel;

   // Construye la clase para background substraction
   bgModel = BackgroundSubtractorMOG2();

   // Usado para guardar imágenes
   int n = 0;
   char filename[200];

   for (;;) {
       capture >> frame;
       if( !frame.empty() )
       {

           mascara=detectForeground( frame , Mask , bgModel);
           //Reduce el tamaño de ventana

           resize(frame, original, Size(), 0.5, 0.5, INTER_LINEAR);
       }
       else
       { printf(" --(!) No captured frame -- Break!"); break; }

       // Retarda N millis, suficiente para mostrar y capturar comandos
       char key = (char)waitKey(5);
       switch (key) {
       case 'q':
       case 'Q':
       case 27: //escape key
            return 0;
       case ' ': //Guarda una imagen
            sprintf(filename,"filename%.3d.jpg",n++);
            imwrite(filename,frame);
            cout << "Guardada " << filename << endl;
            break;
       case 'p': //Pausa
            waitKey();
            break;
       case 'l':
            sliders(frame);
       default:
            break;
       }
   }
   return 0;
 }

/** @function detectForeground
    Enmascara la imagen*/
 Mat detectForeground(Mat frame , Mat& Mask, BackgroundSubtractorMOG2& bgModel)
 {
     Mat mascara;
     std::vector<std::vector<cv::Point> > contours;
     bgModel(frame , Mask , -1);

     // Eliminamos ruido
     erode(Mask,Mask,cv::Mat());
     dilate(Mask,Mask,cv::Mat());

     // Rellena huecos
     Mat element = getStructuringElement( MORPH_ELLIPSE,
                                          Size( 4*morph_size + 1, 4*morph_size+1 ),
                                          Point( morph_size, morph_size ) );
     dilate(Mask,Mask,element);
     //erode(Mask,Mask,element);

     //Reduce tamaño de ventana
     resize(Mask, mascara, Size(), 0.5, 0.5, INTER_LINEAR);
     string window_name = "Foreground mask: SPACE to save a frame, q or esc to quit";
     imshow( window_name, mascara==255);// get rid of shadows

     /*
     //Aplica la máscara sobre la imagen
     Mat frameAux;
     frame.copyTo(frameAux,Mask);

     //Reduce tamaño de ventana
     resize(frameAux, frameAux, Size(), 0.5, 0.5, INTER_LINEAR);
     imshow("foreground",frameAux);
*/
     //Obtenemos los objetos de la máscara
     Mat img=detecta_contorno(Mask==255, frame);

     return Mask;
 }

 /** @function detecta_contorno
  * Busca los objetos a partir
  * de los bordes que encuentre
  * en la máscara, y los encuadra **/

 Mat detecta_contorno(Mat img, Mat frame){
     Mat frameAux;
     vector<vector<Point> > contours;
     findContours(img, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
     // Dibuja los objetos en blanco
     drawContours(img, contours,-1,0,-1,8,noArray(),255,Point());

     // Encuadramos los objetos obtenidos
     vector<vector<Point> > contours_poly(contours.size());
     vector<Rect> boundRect(contours.size());
     for(int i=0; i<contours.size(); i++){
         approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
         boundRect[i] = boundingRect(Mat(contours_poly[i]));
         //rectangle(frame, boundRect[i], 100, 2, 8, 0);
     }
     histograma(boundRect, frame);
     //Reduce tamaño de ventana
     resize(frame, frameAux, Size(), 0.5, 0.5, INTER_LINEAR);
     imshow("Contornos", frameAux);
     return img;
 }

vector<Mat> histograma(vector<Rect> boundRect, Mat frame){
    vector<Mat> rectInterestedVector;
    Mat rectSection;
    Mat rectSectionHSV;
    Mat rectInterested;
    vector<vector<Point> > contours;
    Point2i punto;

    for (int i=0; i<boundRect.size();i++){
        rectSection=frame(boundRect[i]);
        cvtColor(rectSection, rectSectionHSV, COLOR_BGR2HSV);
        inRange(rectSectionHSV, Scalar(0,125,127), Scalar(255,255,255),rectInterested);
        erode(rectInterested, rectInterested, getStructuringElement(MORPH_ELLIPSE, Size(5,5)));
        findContours(rectInterested, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
        if(contours.size()!=0){
            rectInterestedVector.push_back(rectInterested);
            rectangle(frame, boundRect[i], 100, 2, 8, 0);      
            //punto=angulo(boundRect, i, frame);
        }
    }
        return rectInterestedVector;
}

Point2i angulo(vector<Rect> boundRect, int i, Mat frame){
    Rect coord = boundRect.at(i);
    cout << "Propiedades del rectangulo" << coord << endl;
    cout << "Ancho x Alto: " << coord.width << ":" << coord.height << endl;
    cout << "Coordenadas esquina: " << coord.x << ":" << coord.y << endl;
    int x = coord.x + coord.width/2;
    int y = coord.y + coord.height/2;
    Point2i punto;
    punto.x = x;
    punto.y = y;
    cout << punto << endl;
    int difX = punto.x-frame.cols/2;
    int anguloX = (difX*40)/(frame.cols/2);
    int difY = -(punto.y-frame.rows/2);
    int anguloY = (difY*40)/(frame.rows/2);
    cout << "Tamano de escena: " << frame.cols << ":" << frame.rows << endl;
    cout << "Angulos: " << anguloX << ":" << anguloY << endl;
    waitKey();
    return punto;
}

void sliders(Mat mascara){
    namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"
    int iLowH = 0;
    int iHighH = 179;

     int iLowS = 0;
    int iHighS = 255;

     int iLowV = 0;
    int iHighV = 255;

     //Create trackbars in "Control" window
    cvCreateTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
    cvCreateTrackbar("HighH", "Control", &iHighH, 179);

     cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
    cvCreateTrackbar("HighS", "Control", &iHighS, 255);

     cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
    cvCreateTrackbar("HighV", "Control", &iHighV, 255);
while(true){
    Mat imgHSV;

      cvtColor(mascara, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

     Mat imgThresholded;

      inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

     //morphological opening (remove small objects from the foreground)
     erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
     dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

      //morphological closing (fill small holes in the foreground)
     dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
     erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

      imshow("Thresholded Image", imgThresholded); //show the thresholded image
      if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
             {
                  cout << "esc key is pressed by user" << endl;
                  break;
             }
}
      return;
}
