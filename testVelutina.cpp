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
//Mat detectForeground( Mat frame , Mat& Mask , BackgroundSubtractorMOG2& bgModel);
vector<Rect> detecta_contorno(Mat mascara, Mat frame);
//vector<Point2i> histograma(vector<Rect> boundRect, Mat frame);
vector<Rect> comp_Rect(vector<Rect> amarillo, Rect boundRect);
vector<Point2i> hist_no_mov(Mat frame);
Point2i angulo(Rect coord, Mat frame);
int sendCoords(int X, int Y,int Z);

/** Variables globales */

/** Ruta para la carpeta de la tubería FIFO */
string DEFAULT_PIPE_NAME = "/tmp/coordenadas";

string window_name = "Capture - Face detection";
static int Xmin=0, Ymin=0, Xmax=0, Ymax=0; //Encuadrar
const int MAX_COUNT = 500;
const int MAX_ITER = 10;
int morph_size=5;
bool trackObject=false;
bool grabar=false;
int bins = 16;
int Lmin = 30;
int Lmax = 200;
float distancia;
cv::MatND hist(2, bins, CV_8UC2);
vector<KeyPoint> keypointsFace,keypointsFrame;
Mat grabacion;
int opcion;

 /** @function main */
 int main( int argc, const char** argv )
 {
   VideoWriter output_video;
   Mat img, frame, Mask, mascara, original, cuadra;
   vector<Rect> boundRect;
   vector<Point2i> puntos;

   //Para vídeo o cámara por defecto
  /* std::string arg = argv[1];

   // Abre el archivo de entrada
   VideoCapture capture(arg);

   // Si falla, activa la cámara por defecto
    if (!capture.isOpened())
        capture.open(atoi(arg.c_str()));
    if (!capture.isOpened()) {
        cerr << "No se ha podido abrir el archivo ni activar la cámara\n" << endl;
        return 1;
    }*/


   //Cámara secundaria
   VideoCapture capture(0);
   if (!capture.isOpened()) {
       cerr << "Failed to open a video device or video file!\n" << endl;
       return 1;
   }

   // Obtiene los FPS
   double fps = capture.get(CV_CAP_PROP_FPS);
   cout << "Frames per second: " << fps << endl;
   BackgroundSubtractorMOG2 bgModel;

   // Construye la clase para background substraction
   bgModel = BackgroundSubtractorMOG2();
   // Usado para guardar imágenes
   //int n = 0;
   //char filename[200];

   // Elegir opcion
   //cout << "Detectar movimiento: teclea 1 para SI, otro numero para NO: " << endl;
   //cin >> opcion;
  // cout << endl;
   //int i=0;
   for (;;) {
       capture >> frame;
       if( !frame.empty() )
       {
          // if(opcion==1)
          // {
           //Con detect foreground
        /*   mascara=detectForeground( frame , Mask , bgModel);
           boundRect=detecta_contorno(mascara==255, frame);
           if(i%2==0)
           {
               puntos=histograma(boundRect, frame);
               i++;
           }
           else
           {
               i++;
           }
           video.write(grabacion);
           if(puntos.size()>0)
           {
               //waitKey();
           /*    for(int x=0;x<puntos.size();x++)
               {
                   sendCoords(puntos[x].x,puntos[x].y,distancia);
               }
           }
          /* if(grabar==true)
           {
            video.write(frame);
           }
           video.write(frame);*/
        //   }
          // else{
           //Sin detect foreground
           puntos=hist_no_mov(frame);
           for(int x=0;x<puntos.size();x++)
           {
	    cout << "CoodenadaX:" << puntos[x].x << " - CoordenadaY:" << puntos[x].y << " - Distancia:" << distancia << endl;
            sendCoords(puntos[x].x,puntos[x].y,distancia);
           }
            //video.write(grabacion);

        //    }
           resize(frame, original, Size(), 0.5, 0.5, INTER_LINEAR);
       }
       else
       {
           printf(" --(!) No captured frame -- Break!"); break;
       }

       // Retarda N millis, suficiente para mostrar y capturar comandos
       char key = (char)waitKey(5);
       switch (key)
       {
       case 'q':
       case 'Q':
       case 27: //escape key
            return 0;
       default:
            break;
       }
   }
   return 0;
 }

/** @function detectForeground
    Enmascara la imagen*/
 /*Mat detectForeground(Mat frame , Mat& Mask, BackgroundSubtractorMOG2& bgModel)
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
    // imshow( window_name, mascara==255);// get rid of shadows

     /*
     //Aplica la máscara sobre la imagen
     Mat frameAux;
     frame.copyTo(frameAux,Mask);
     //Reduce tamaño de ventana
     resize(frameAux, frameAux, Size(), 0.5, 0.5, INTER_LINEAR);
     imshow("foreground",frameAux);

     //Obtenemos los objetos de la máscara


     return Mask;
 }*/

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
    // imshow("Contornos", frameAux);
     return boundRect;
 }

/*vector<Point2i> histograma(vector<Rect> boundRect, Mat frame){

    Mat frameAux;
    Mat rectSection;
    Mat rectSectionHSV;
    Mat verde;
    Mat negro;
    Mat rectInterested;
    vector<vector<Point> > contours,contours2,contours3;
    vector<Rect> amarillo;
    Point2i punto;
    vector<Point2i> puntos;

    //Visualizar imagen HSV
    Mat frameHSV;
    cvtColor(frame, frameHSV, COLOR_BGR2HSV);
   // imshow("HSV", frameHSV);

    for (int i=0; i<boundRect.size();i++){
        rectSection=frame(boundRect[i]);
//        cvtColor(rectSection, rectSectionHSV, COLOR_BGR2HSV);
        inRange(rectSection, Scalar(0,64,160), Scalar(60,123,255),rectInterested);
        inRange(rectSection,Scalar(0,1,0),Scalar(32,32,30),negro);
        //erode(rectInterested, rectInterested, getStructuringElement(MORPH_ELLIPSE, Size(5,4)));
        findContours(rectInterested, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
        findContours(negro, contours2, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
        if(contours.size()!=0 && contours2.size()!=0)
        {
            amarillo = comp_Rect(amarillo, boundRect[i]);
            //rectangle(frame, boundRect[i], 100, 2, 8, 0);
        }

    }
    for(int i=0; i<amarillo.size(); i++){
        punto=angulo(amarillo[i], frame);
        puntos.push_back(punto);
        rectangle(frame, amarillo[i], 100, 2, 8, 0);
    }
//    resize(frame, frameAux, Size(), 0.5, 0.5, INTER_LINEAR);
    grabacion=frame.clone();
    resize(frame, frame, Size(), 0.5, 0.5, INTER_LINEAR);
    imshow("Seccion por color", frame);
        return puntos;
}*/

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
    float area=coord.area();
    float area_avispa=6.25;
    float k=290485.47;
    int x = coord.x;
    int y = coord.y;
    Point2i punto;
    punto.x = x;
    punto.y = y;
    distancia=sqrt((area_avispa*k)/area);
    return punto;
}


vector<Point2i> hist_no_mov(Mat frame){

    Mat frameAux;
    Mat rectSection;
    Mat rectSectionHSV;
    Mat rectInterested;
    Mat negro;
    vector<vector<Point> > contours;
    vector<Rect> amarillo;
    Point2i punto;
    vector<Point2i> puntos;
    vector<Rect> boundRect;
    Rect turno;
        frameAux=frame.clone();
 //       cvtColor(frame, rectSectionHSV, COLOR_BGR2HSV);
        inRange(frame, Scalar(0,54,160), Scalar(52,123,255),rectInterested);
        imshow("mascara",rectInterested);
        dilate(rectInterested,rectInterested, getStructuringElement(MORPH_ELLIPSE, Size(5,5)));
        boundRect=detecta_contorno(rectInterested, frame);
        for(int i=0; i<boundRect.size();i++)
        {
            /*turno=boundRect[i];
            turno.x=turno.x-30;
            turno.y=turno.y-30;
            turno.height=turno.height+60;
            turno.width=turno.width+60;
            amarillo = comp_Rect(amarillo, turno);*/
	    rectSection=frame(boundRect[i]);
            inRange(rectSection,Scalar(0,1,0),Scalar(32,32,30),negro);
	    findContours(negro,contours,CV_RETR_LIST,CV_CHAIN_APPROX_SIMPLE);
	    if(contours.size()!=0)
	    {
            	amarillo = comp_Rect(amarillo, boundRect[i]);
	    }
        }

        for(int i=0; i<amarillo.size(); i++){
            punto=angulo(amarillo[i], frame);
            puntos.push_back(punto);
            rectangle(frameAux, amarillo[i], 100, 2, 8, 0);
        }
        resize(frameAux, frameAux, Size(), 0.5, 0.5, INTER_LINEAR);
        grabacion=frameAux.clone();
        imshow("Seccion por color", frameAux);
        return puntos;
}


/*
Envia las coordenadas X e Y mediante tuberias a otro proceso distinto que las interpretará adecuadamente
Added by brunonogareda
*/
int sendCoords(int X, int Y, int Z) {

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
      char msg[20] = "";
      snprintf(msg, sizeof(msg), "X:%i-Y:%i-Z:%i-",X,Y,Z);
      write(t,msg,sizeof(msg));

      //cerramos la tuberia
      close(t);

      //borramos
      //unlink(pipe);

       return 0;
}
