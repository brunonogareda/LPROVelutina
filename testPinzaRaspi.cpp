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
vector<Rect> comp_Rect(vector<Rect> rectangulos, Rect boundRect);
vector<Point2i> hist_no_mov(Mat frame);
Point2i coordToPoint(Rect coord, Mat frame);
int sendCoords(int X, int Y);


/** Variables globales */

/** Ruta para la carpeta de la tubería FIFO */
string DEFAULT_PIPE_NAME = "/tmp/coordenadas";

/** Rango de detecci�n de colores */
Scalar RangeColorDetec1 = Scalar(90,192,117);     //Pinza Azul
Scalar RangeColorDetec2 = Scalar(112,255,217);		//Pinza Azul
//Scalar RangeColorDetec1 = Scalar(122,57,148);		//Poliespan Rosa
//Scalar RangeColorDetec2 = Scalar(179,134,233);	//Poliespan Rosa

Scalar ColorRectangle = Scalar(17, 255, 0);


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
   VideoCapture capture(0);

   // Si falla, activa la cámara por defecto
    if (!capture.isOpened())
        //capture.open(atoi(arg.c_str()));
        capture.open(0);
    if (!capture.isOpened()) {
        cerr << "No se ha podido abrir el archivo ni activar la cámara\n" << endl;
        return 1;
    }

   //int ex = static_cast<int>(capture.get(CV_CAP_PROP_FOURCC));

   int ex = 1;
   output_video.open("/tmp/x", ex, 15, Size(capture.get(CV_CAP_PROP_FRAME_WIDTH), capture.get(CV_CAP_PROP_FRAME_HEIGHT)), true);

   // Obtiene los FPS
   //capture.set(CV_CAP_PROP_FPS, 60);
   //double fps = capture.get(CV_CAP_PROP_FPS);
   double fps = 15;
   cout << "Frames per second: " << fps << endl;

   // Usado para guardar imágenes
   int n = 0;
   char filename[200];
   for (;;) {
       capture >> frame;
       output_video << frame;
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
            break;
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
     //resize(frame, frameAux, Size(), 0.75, 0.75, INTER_LINEAR);
     //imshow("Contornos", frameAux);
     return boundRect;
 }

vector<Rect> comp_Rect(vector<Rect> rectangulos, Rect boundRect){
    if(rectangulos.empty()==true){
        rectangulos.push_back(boundRect);
    }
    else{
        Rect comprobar;
        Rect fusion;
        for(int i=0; i<rectangulos.size(); i++){
            comprobar = rectangulos.at(i);
            //Si el nuevo Rect esta dentro del antiguo
            //if(comprobar.x < minX && comprobar.x > maxX && comprobar.y < minY && comprobar.y > maxY){}
            if((boundRect & comprobar).area()!=0){
            fusion = boundRect | comprobar;
            rectangulos.at(i) = fusion;
            }
            else if(i == rectangulos.size()-1){
                rectangulos.push_back(boundRect);
            }
        }
    }
    return rectangulos;
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
    vector<Rect> deteccion;
    Point2i punto;
    vector<Point2i> puntos;
    vector<Rect> boundRect;

        cvtColor(frame, rectSectionHSV, COLOR_BGR2HSV);
        inRange(rectSectionHSV, RangeColorDetec1, RangeColorDetec2,rectInterested);
        //dilate(rectInterested,rectInterested, getStructuringElement(MORPH_ELLIPSE, Size(10,10)));
        boundRect=detecta_contorno(rectInterested, frame);
        for(int i=0; i<boundRect.size();i++){
            deteccion = comp_Rect(deteccion, boundRect[i]);
        }

        for(int i=0; i<deteccion.size(); i++){
            punto=coordToPoint(deteccion[i], frame);
            puntos.push_back(punto);
            rectangle(frame, deteccion[i], ColorRectangle, 2, 8, 0);
        }
    resize(frame, frameAux, Size(), 0.75, 0.75, INTER_LINEAR);
    imshow("Seccion por color", frameAux);
    return puntos;
}

/*
Envia las coordenadas X e Y mediante tuberias a otro proceso distinto que las interpretará adecuadamente
Added by brunonogareda
*/
int sendCoords(int X, int Y) {

      int t;
      char * pipe = new char[DEFAULT_PIPE_NAME.length()+1];
      strcpy(pipe, DEFAULT_PIPE_NAME.c_str());

      /*crea un nuevo archivo fifo especial,
      incluye una ruta y un parametro con una
      mascara de permisos.
      */
      mkfifo(pipe,0666);

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
