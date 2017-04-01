#include<cv.h>
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/video/tracking.hpp"
#include "opencv2/nonfree/nonfree.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <stdio.h>
#include<string.h>
#include <Windows.h>
 using namespace std;
 using namespace cv;

 /** Funciones */
Mat detectForeground( Mat frame , Mat& Mask , BackgroundSubtractorMOG2& bgModel);
vector<Rect> detecta_contorno(Mat mascara, Mat frame);
vector<Point2i> histograma(vector<Rect> boundRect, Mat frame);
vector<Rect> comp_Rect(vector<Rect> amarillo, Rect boundRect);
vector<Point2i> hist_no_mov(Mat frame);
Point2i angulo(Rect coord, Mat frame);
int sendCoords(int X, int Y);
void sliders(Mat mascara);
wstring s2ws(const std::string& s);
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
   vector<Rect> boundRect;
   vector<Point2i> puntos;

   //Para vídeo o cámara por defecto
   //std::string arg = argv[1];

   // Abre el archivo de entrada
   VideoCapture capture(1);

   // Si falla, activa la cámara por defecto
    if (!capture.isOpened())
        //capture.open(atoi(arg.c_str()));
        capture.open(1);
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
           //Con detect foreground
           //mascara=detectForeground( frame , Mask , bgModel);
           //boundRect=detecta_contorno(mascara==255, frame);
           //puntos=histograma(boundRect, frame);

           //Sin detect foreground
           puntos=hist_no_mov(frame);

           resize(frame, original, Size(1280,720), 0.5, 0.5, INTER_LINEAR);
           for(int i=0;i<puntos.size();i++)
           {
               sendCoords(puntos[i].x,puntos[i].y);
           }

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
       case 'c':
            cout << "Coordenadas obtenidas: " << puntos << endl;
            waitKey();
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


     return Mask;
 }

 /** @function detecta_contorno
  * Busca los objetos a partir
  * de los bordes que encuentre
  * en la máscara, y los encuadra **/

 vector<Rect> detecta_contorno(Mat mascara, Mat frame){
     Mat frameAux;
     vector<vector<Point> > contours;
     findContours(mascara, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
     // Dibuja los objetos en blanco
     drawContours(mascara, contours,-1,0,-1,8,noArray(),255,Point());

     // Encuadramos los objetos obtenidos
     vector<vector<Point> > contours_poly(contours.size());
     vector<Rect> boundRect(contours.size());
     for(int i=0; i<contours.size(); i++){
         approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
         boundRect[i] = boundingRect(Mat(contours_poly[i]));
         //rectangle(frame, boundRect[i], 100, 2, 8, 0);
     }
     //Reduce tamaño de ventana
     resize(frame, frameAux, Size(), 0.5, 0.5, INTER_LINEAR);
     imshow("Contornos", frameAux);
     return boundRect;
 }

vector<Point2i> histograma(vector<Rect> boundRect, Mat frame){

    Mat frameAux;
    Mat rectSection;
    Mat rectSectionHSV;
    Mat rectInterested;
    vector<vector<Point> > contours;
    vector<Rect> amarillo;
    Point2i punto;
    vector<Point2i> puntos;

    for (int i=0; i<boundRect.size();i++){
        rectSection=frame(boundRect[i]);
        cvtColor(rectSection, rectSectionHSV, COLOR_BGR2HSV);
        inRange(rectSectionHSV, Scalar(0,125,127), Scalar(255,255,255),rectInterested);
        erode(rectInterested, rectInterested, getStructuringElement(MORPH_ELLIPSE, Size(5,5)));
        findContours(rectInterested, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
        if(contours.size()!=0){
            amarillo = comp_Rect(amarillo, boundRect[i]);
            //rectangle(frame, boundRect[i], 100, 2, 8, 0);

        }

    }
    for(int i=0; i<amarillo.size(); i++){
        punto=angulo(amarillo[i], frame);
        puntos.push_back(punto);
        rectangle(frame, amarillo[i], 100, 2, 8, 0);
    }
    resize(frame, frameAux, Size(), 0.5, 0.5, INTER_LINEAR);
    imshow("Seccion por color", frameAux);
        return puntos;
}

vector<Rect> comp_Rect(vector<Rect> amarillo, Rect boundRect){
    if(amarillo.empty()==true){
        amarillo.push_back(boundRect);
    }
    else{
       /* int minX = boundRect.x;
        int maxX = minX + boundRect.width;
        int minY = boundRect.y;
        int maxY = minY + boundRect.height;
        int X2, Y2;
        */
        Rect comprobar;
        Rect fusion;
        for(int i=0; i<amarillo.size(); i++){
            comprobar = amarillo.at(i);
            //Si el nuevo Rect esta dentro del antiguo
            //if(comprobar.x < minX && comprobar.x > maxX && comprobar.y < minY && comprobar.y > maxY){}
            if((boundRect & comprobar).area()!=0){
            fusion = boundRect | comprobar;
            amarillo.at(i) = fusion;
            }
            else if(i == amarillo.size()-1){
                amarillo.push_back(boundRect);
            }
        }
    }
    return amarillo;
}


Point2i angulo(Rect coord, Mat frame){
    int x = coord.x;
    int y = coord.y;
    Point2i punto;
    punto.x = x;
    punto.y = y;
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


vector<Point2i> hist_no_mov(Mat frame){

    Mat frameAux;
    Mat rectSection;
    Mat rectSectionHSV;
    Mat rectInterested;
    vector<vector<Point> > contours;
    vector<Rect> amarillo;
    Point2i punto;
    vector<Point2i> puntos;
    vector<Rect> boundRect;

        cvtColor(frame, rectSectionHSV, COLOR_BGR2HSV);
        inRange(rectSectionHSV, Scalar(90,192,117), Scalar(112,255,217),rectInterested);
        dilate(rectInterested,rectInterested, getStructuringElement(MORPH_ELLIPSE, Size(5,5)));
        boundRect=detecta_contorno(rectInterested, frame);
        for(int i=0; i<boundRect.size();i++){
            amarillo = comp_Rect(amarillo, boundRect[i]);
        }

        for(int i=0; i<amarillo.size(); i++){
            punto=angulo(amarillo[i], frame);
            puntos.push_back(punto);
            rectangle(frame, amarillo[i], 100, 2, 8, 0);
        }
    resize(frame, frameAux, Size(), 0.5, 0.5, INTER_LINEAR);
    imshow("Seccion por color", frameAux);
        return puntos;
}
int sendCoords(int X, int Y)
{
    HANDLE hPipe;
    DWORD dwWritten;
    //char *pipe = "/tmp/coordenadas";
    String ruta_pipe="Z:\\coordenadas";
    String coords="X:"+to_string(X)+"-Y:"+to_string(Y)+"-";
    WString aux=s2ws(ruta_pipe);
    LPCWSTR pipe = aux.c_str();
    LPCVOID datos=coords.c_str();
    //cout<<X<<","<<Y<<endl;
    //cout<<datos<<endl;
    cout<<coords<<endl;
    hPipe = CreateFile(pipe,GENERIC_WRITE,0,NULL,OPEN_EXISTING,0, NULL);
        if (hPipe != INVALID_HANDLE_VALUE)
        {
            WriteFile(hPipe,datos,coords.size(),&dwWritten,NULL);
            CloseHandle(hPipe);
        }

        return (0);
}
wstring s2ws(const std::string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}
