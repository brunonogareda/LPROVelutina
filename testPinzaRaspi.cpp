#include<cv.h>
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/video/tracking.hpp"
//#include "opencv2/nonfree/nonfree.hpp"

#include <iostream>
#include <stdio.h>
#include<string.h>

//Librerias para comunicación entre procesos
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


 using namespace std;
 using namespace cv;

 /** Funciones */
vector<Rect> detecta_contorno(Mat mascara, Mat frame);
vector<Rect> comp_Rect(vector<Rect> amarillo, Rect boundRect);
vector<Point2i> hist_no_mov(Mat frame);
Point2i coordToPoint(Rect coord, Mat frame);
int sendCoords(int X, int Y);


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
string DEFAULT_PIPE_NAME = "/tmp/coordenadas";

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
   VideoCapture capture(-1);

   // Si falla, activa la cámara por defecto
    if (!capture.isOpened())
        //capture.open(atoi(arg.c_str()));
        capture.open(0);
    if (!capture.isOpened()) {
        cerr << "No se ha podido abrir el archivo ni activar la cámara\n" << endl;
        return 1;
    }

   // Obtiene los FPS
   double fps = 15;//capture.get(CV_CAP_PROP_FPS);
   cout << "Frames per second: " << fps << endl;

   // Usado para guardar imágenes
   int n = 0;
   char filename[200];
   for (;;) {
       capture >> frame;
       if( !frame.empty() )
       {

           //Sin detect foreground
           puntos=hist_no_mov(frame);

           resize(frame, original, Size(1280,720), 0.5, 0.5, INTER_LINEAR);
           for(int i=0;i<puntos.size();i++)
           {
		          cout << puntos[i] << endl;
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
       case 'c':
            cout << "Coordenadas obtenidas: " << puntos << endl;
            waitKey();
       default:
            break;
       }
   }
   return 0;
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
     //imshow("Contornos", frameAux);
     return boundRect;
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


Point2i coordToPoint(Rect coord, Mat frame){
    int x = coord.x;
    int y = coord.y;
    Point2i punto;
    punto.x = x;
    punto.y = y;
    return punto;
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
            punto=coordToPoint(amarillo[i], frame);
            puntos.push_back(punto);
            rectangle(frame, amarillo[i], 100, 2, 8, 0);
        }
    resize(frame, frameAux, Size(), 0.5, 0.5, INTER_LINEAR);
    //imshow("Seccion por color", frameAux);
        return puntos;
}

/*
Envia las coordenadas X e Y mediante tuberias a otro proceso distinto que las interpretará adecuadamente
Added by brunonogareda
*/
int sendCoords(int X, int Y) {

      int t;
      char *pipe;
      strcpy(pipe, DEFAULT_PIPE_NAME.c_str());

      /*crea un nuevo archivo fifo especial,
      incluye una ruta y un parametro con una
      mascara de permisos.
      */
      mkfifo(pipe,0666);

      /*
      abrimos una nueva tuberia
      O_RDONLY - Abrir para solo lectura
      O_WRONLY - Abrir para solo escritura
      O_RDWR - Abrir para lectura / escritura
      O_APPEND - Agrega al final del Archivo
      ...
      */
      t = open(pipe,O_WRONLY | O_NONBLOCK);
      //escribimos el mensaje que compartiremos
      char msg[40] = "";
      snprintf(msg, sizeof(msg), "X:%i-Y:%i", X,Y);
      write(t,msg,sizeof(msg));

      //cerramos la tuberia
      //close(t);

      //borramos
     	//unlink(pipe);

       return 0;
}
