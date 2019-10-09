/*
 * catalogador.cpp

 *
 *  Created on: 05/03/2016
 *      Author: Diana
 */

#include <imgproc.hpp>
#include <highgui.hpp>
#include <core.hpp>
#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <math.h>
#include <imgcodecs.hpp>
#include <sstream>
#include <windows.h>

using namespace cv;
using namespace std;

String img_nombre, orig;
String modificacion;

Mat src, img, src_gray;
int thresh = 100;
int max_thresh = 255;
int numImg;
RNG rng(12345);
char * ruta;
char * area;
char * perimetro;
char * fechaf;
char cadena[5000];
char cadenaF[5000];
String Cad= "";

//Strings para nombrar a las imágenes al momento de guardar
String forma= "FormaImg ";
String centroide= "CentroideImg ";
String esq= "EsqueletoImg ";
String convexH= "ConvexImg ";
String gaborI= "GaborImg ";
String cadenaI= "CadenaImg ";
String firmaI= "FirmaImg ";

char* variable;

//Variables para el filtro de Gabor
int kernel_size=21;
int pos_sigma= 5;
int pos_lm = 50;
int pos_th = 0;
int pos_psi = 90;
cv::Mat src_f;
cv::Mat dest;

//Coordenadas del centroide para el cálculo de la firma
int cx, cy;

/// Función de la cabecera
int modificacionFecha(String archivo);
void escribirFichero(char * name, char * auxRes1, char * auxRes2, char * auxTam1, char * auxTam2, char * auxFormato);
void thresh_callback(int, void* );
void esqueleto();
void convex();
cv::Mat mkKernel(int ks, double sig, double th, double lm, double ps);
void GaborProcess(int , void *);
void Ccadena();
void firma();


int main()
{

	//Mat img;

	//Se obtiene la ruta de los archivos

	char exepath[MAX_PATH];
	GetModuleFileName(0, exepath, MAX_PATH);
	//cout << exepath << "\n";

	//Se convierte la variable tipo char obtenida para la ruta a una de tipo String
	string variable;
	stringstream conversionVar;
	conversionVar << exepath;
	variable = conversionVar.str();

	//Como la ruta se obtiene del ejecutable, obtengo un substring para capturar el path hasta la carpeta general del proyecto
	std::string rutaStr = variable.substr(0, 36);
	std::cout << rutaStr << '\n';

	//Convierto el substring obtenido a un char* para almacenarlo en el fichero
	ruta= strdup(rutaStr.c_str());


	int var=0, cont=0, cont2=1;

	//String img_nombre, orig;
	String ext[5]={".png", ".jpg", ".bmp", ".gif", ".pic"};

	//orig= "C:\\Cuadrado2.";
	//FILE *imagen;
	//imagen= fopen("perrito.jpg", "r");


	//Se solicita la cantidad de imágenes a procesar
	cout << "\nIndique el número de imágenes a procesar:"<<endl;
	cin>>numImg;

	for(int i=0; i<numImg; i++)
	{
		string nombre;
		stringstream conversion; //Se usa para la conversión
		conversion << cont2;
		nombre = conversion.str();

		orig= nombre;
		printf("\nNombre: %s\n", nombre.c_str());

		while(var==0)
		{
			img_nombre= orig.c_str();
			img_nombre+=ext[cont];


			cout<<"La imagen es: "<<img_nombre<<endl;

			img = imread(img_nombre, CV_LOAD_IMAGE_COLOR);
			//**************
			src= img.clone();


			if(! img.data )                              // Check for invalid input
			{
				cout <<  "No se encontró la imagen" << std::endl ;
				cont++;

				//return -1;
			}

			else
			{
				/*printf("El tamaño de la imagen es de: %d", img.size);
				namedWindow("MyWindow", CV_WINDOW_AUTOSIZE);
				imshow("MyWindow", img);*/

				var=1;
			}

		}


//**************************************DESCRIPTORES**********************************************************

		//Forma
		cvtColor( src, src_gray, CV_BGR2GRAY );
		blur( src_gray, src_gray, Size(3,3) );
		thresh_callback( 0, 0 );

		//Esqueleto
		esqueleto();

		//Convex Hull
		convex();

		//Filtro de Gabor
	    cv::cvtColor(img, src, CV_BGR2GRAY);
	    src.convertTo(src_f, CV_32F, 1.0/255, 0);
	    if (!kernel_size%2)
	    {
	    	kernel_size+=1;
	    }
	    GaborProcess(0,0);

	    //Código de Cadena
	    Ccadena();

	    //Firma
	    firma();

//*****************************************************************************************************


		//Convertir img_nombre de String a *name
		char *name = strdup(img_nombre.c_str());


		//Creo un fichero para estimar el tamaño del archivo de imagen
		FILE *imagen;
		imagen= fopen(name, "r");
		fseek(imagen, 0L, SEEK_END); // Colocar el cursor al final del fichero
		long nBytes = ftell(imagen); // Obtengo en Bytes el tamaño del archivo


		//printf("El tamaño de la imagen es de: %d", img.elemSize());

		int res1, res2, tam1, formato;
		float tam2;

		res1= img.cols;
		res2= img.rows;

		tam1= img.total()*img.elemSize();
		tam1= tam1/1000;
		tam2= (int) nBytes;
		tam2= tam2/1000;
		char *auxFormat = strdup(ext[cont].c_str());
		//formato= (string)ext[cont];


		//Conversión de los enteros a string y posteriormente en char* para almacenarlo en el fichero

		string result1;//string which will contain the result
		stringstream convert; // stringstream used for the conversion
		convert << res1;//add the value of Number to the characters in the stream
		result1 = convert.str();//set Result to the content of the stream
		char *auxRes1 = strdup(result1.c_str());


		string result2;
		stringstream convert2;
		convert2 << res2;
		result2 = convert2.str();
		char *auxRes2 = strdup(result2.c_str());

		string result3;
		stringstream convert3;
		convert3 << tam1;
		result3 = convert3.str();
		char *auxTam1 = strdup(result3.c_str());

		string result4;
		stringstream convert4;
		convert4 << tam2;
		result4 = convert4.str();
		char *auxTam2 = strdup(result4.c_str());

		string result5;
		stringstream convert5;
		convert5 << auxFormat;
		result5 = convert5.str();
		char *auxFormato = strdup(result5.c_str());



		printf("\nLa resolucion de la imagen es de %dx%d", img.cols, img.rows);
		printf("\nEl tamaño aproximado de la imagen en bytes es de: %d", img.total()*img.elemSize());
		printf("\nEl tamaño aproximado de la imagen en bytes usando la lectura de fichero es de: %d", nBytes);
		cout << "\nEl formato de la imagen es: " << ext[cont] <<endl;
		/*namedWindow("MyWindow", CV_WINDOW_AUTOSIZE);
		imshow("MyWindow", img);*/


		modificacionFecha(img_nombre);

		//Mando los datos al fichero
		escribirFichero(name, auxRes1, auxRes2, auxTam1, auxTam2, auxFormato);


		img_nombre= "";
		cont2++;//Paso a la siguiente imagen
		var=0;
		cont=0;
	}

	printf("\n\nSe ha terminado exitosamente!!!, por favor proceda a abrir el archivo 'catalogador.html' para visualizar la tabla");

	waitKey(0);
	return 0;
}


void escribirFichero(char * name, char * auxRes1, char * auxRes2, char * auxTam1, char * auxTam2, char * auxFormato)
{
	//Creo el fichero con formato html para generar la tabla

	FILE *fichero;

	fichero= fopen("catalogador.html", "r+");

	/*Coloco el puntero al final del fichero para que no elimine lo que se encuentra al inicio del fichero
	 * que son las cabeceras del formato html y de la tabla.
	 */

	fseek(fichero, 0L, SEEK_END);


	//Escribo en el fichero la etiqueta html <td> para que ponga en cada columna los respectivos datos

	//strdup(ext[cont].c_str());

	//Convierto los nombres de las imágenes generadas por los descriptores a char* para almacenarlas al catalogador
	char * imagen= strdup(img_nombre.c_str());
	char * dForma= strdup(forma.c_str());
	char * desq= strdup(esq.c_str());
	char * dconvex= strdup(convexH.c_str());
	char * dgabor= strdup(gaborI.c_str());
	char * dcadena= strdup(cadenaI.c_str());
	char * dfirma= strdup(firmaI.c_str());

	fputs("<tr class='alt'>", fichero);
	fputs("<td>", fichero);
	fputs("<img src='", fichero);
	fputs(imagen, fichero);
	fputs("' width='150' heigth='150'>", fichero);
	fputs("</td>", fichero);
	fputs("<td>", fichero);
	fputs(name, fichero);
	fputs("</td>", fichero);
	fputs("<td>", fichero);
	fputs(ruta, fichero);
	fputs("</td>", fichero);
	fputs("<td>", fichero);
	fputs(fechaf, fichero);
	fputs("</td>", fichero);
	fputs("<td>", fichero);
	fputs(auxRes1, fichero);
	fputs("x", fichero);
	/*fputs("</td>", fichero);
	fputs("<td>", fichero);*/
	fputs(auxRes2, fichero);
	fputs("</td>", fichero);
	fputs("<td>", fichero);
	fputs(auxTam1, fichero);
	fputs("</td>", fichero);
	fputs("<td>", fichero);
	fputs(auxTam2, fichero);
	fputs("</td>", fichero);
	fputs("<td>", fichero);
	fputs(auxFormato, fichero);
	fputs("</td>", fichero);
	fputs("<td>", fichero);
	//fputs("<img src='Descriptores Forma.png' width='200' heigth='200'>", fichero);
	fputs("<img src='", fichero);
	fputs(dForma, fichero);
	fputs("' width='150' heigth='150'>", fichero);
	fputs("</td>", fichero);
	fputs("<td>", fichero);
	fputs(area, fichero);
	fputs("</td>", fichero);
	fputs("<td>", fichero);
	fputs(perimetro, fichero);
	fputs("</td>", fichero);
	fputs("<td>", fichero);
	//fputs("<img src='Esqueleto.jpg' width='200' heigth='200'>", fichero);
	fputs("<img src='", fichero);
	fputs(desq, fichero);
	fputs("' width='150' heigth='150'>", fichero);
	fputs("</td>", fichero);
	fputs("<td>", fichero);
	//fputs("<img src='Convex.jpg' width='200' heigth='200'>", fichero);
	fputs("<img src='", fichero);
	fputs(dconvex, fichero);
	fputs("' width='150' heigth='150'>", fichero);
	fputs("</td>", fichero);
	fputs("<td>", fichero);
	//fputs("<img src='Gabor.jpg' width='200' heigth='200'>", fichero);
	fputs("<img src='", fichero);
	fputs(dgabor, fichero);
	fputs("' width='150' heigth='150'>", fichero);
	fputs("</td>", fichero);
	fputs("<td>", fichero);
	//fputs("<img src='Cadena.jpg' width='200' heigth='200'>", fichero);
	fputs("<img src='", fichero);
	fputs(dcadena, fichero);
	fputs("' width='150' heigth='150'>", fichero);
	fputs("</td>", fichero);
	fputs("<td>", fichero);
	//fputs("<img src='Firma.png' width='200' heigth='200'>", fichero);
	fputs("<img src='", fichero);
	fputs(dfirma, fichero);
	fputs("' width='150' heigth='150'>", fichero);
	fputs("</td>", fichero);
	/*fputs("<td>", fichero);
	fputs(variable, fichero);
	fputs("</td>", fichero);*/
	fputs("</tr>", fichero);


	//Cierro el fichero
	fclose(fichero);


}



int modificacionFecha(String archivo)
{

	//String archivo= img_nombre;
	char* archivoNombre= strdup(archivo.c_str());
    HANDLE hFile1;
    FILETIME ftCreate;
    SYSTEMTIME stUTC, stLocal;
    hFile1 = CreateFile(archivoNombre, GENERIC_READ, FILE_SHARE_READ,  NULL,  OPEN_EXISTING,  FILE_ATTRIBUTE_NORMAL, NULL);

    if(hFile1 == INVALID_HANDLE_VALUE)
    {
        printf("No se pudo abrir el archivo %ul\n", GetLastError());
        return -1;
    }

    if(!GetFileTime(hFile1, &ftCreate, NULL, NULL))
    {
        printf("Error\n");
        return FALSE;
    }

    FileTimeToSystemTime(&ftCreate, &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
    printf("UTC System Time format:\n");

    int dia, mes, year, hora, min;
    String v1, v2, v3, v4, v5;
    String v1f, v2f, v4f, v5f;
    String fecha;

    if(stUTC.wHour+19>24)
    {
    	printf("Created on: %02d/%02d/%d %02d:%02d\n", stUTC.wDay, stUTC.wMonth, stUTC.wYear, stUTC.wHour-5, stUTC.wMinute);

    	//Obtengo el día, mes, año, hora y minutos de la fecha de modificación del archivo
    	dia= stUTC.wDay;
    	mes= stUTC.wMonth;
    	year= stUTC.wYear;
    	hora= stUTC.wHour-5;
    	min= stUTC.wMinute;


    	//Convierto a Strings los datos obtenidos anteriormente
      	stringstream convertv1; // stringstream used for the conversion
        convertv1 << dia;//add the value of Number to the characters in the stream
        v1 = convertv1.str();//set Result to the content of the stream

        stringstream convertv2;
        convertv2 << mes;
        v2 = convertv2.str();

        stringstream convertv3;
        convertv3 << year;
        v3 = convertv3.str();

        stringstream convertv4;
        convertv4 << hora;
        v4 = convertv4.str();

        stringstream convertv5;
        convertv5 << min;
        v5 = convertv5.str();

        //Si el día, mes, hora o minuto es menor a 10, le agrego un un 0 al principio para darle estilo
        if(dia < 10)
        {
        	v1f= "0";
        	v1f+= v1;
        	//printf("\nValor de V1f Dia: %s", v1f.c_str());
        }
        else
        {
        	v1f= v1;
        }

        if(mes < 10)
        {
        	v2f= "0";
        	v2f+= v2;
        	//printf("\nValor de V2f Mes: %s", v2f.c_str());
        }

        else
        {
        	v2f= v2;
        }

        if(hora < 10)
        {
        	v4f= "0";
        	v4f+= v4;
        	//printf("\nValor de V4f Hora: %s", v4f.c_str());
        }

        else
        {
        	v4f= v4;
        }

        if(min < 10)
        {
        	v5f= "0";
        	v5f+= v5;
        	//printf("\nValor de V5f: %s", v5f.c_str());
        }

        else
        {
        	v5f= v5;
        }


        //Se concatena los datos anteriores en un solo String, dándole un estilo para su correcta visualización
        fecha= v1f.c_str();
        fecha+= "/";
        fecha+= v2f.c_str();
        fecha+= "/";
        fecha+= v3.c_str();
        fecha+= " ";
        fecha+= v4f.c_str();
        fecha+= ":";
        fecha+= v5f.c_str();

    	/*printf("\nModificación: %d %d %d", dia, mes);
    	printf("\nModificación Fecha: %s", v1.c_str());
    	printf("\nModificación Fecha Mes: %s", v2.c_str());
    	printf("\nModificación Fecha Año: %s", v3.c_str());
    	printf("\nModificación Fecha Hora: %s", v4.c_str());
    	printf("\nModificación Fecha Minuto: %s", v5.c_str());
    	printf("\nModificación Fecha Final: %s", fecha.c_str());*/

    	//Una vez generado el String estilizado con todos los datos, lo mando a una variable de tipo char* para almacenarla en el fichero
    	fechaf= strdup(fecha.c_str());
    }


    else
    {
    	printf("Created on: %02d/%02d/%d %02d:%02d\n", stUTC.wDay-1, stUTC.wMonth, stUTC.wYear, stUTC.wHour+19, stUTC.wMinute);
    	dia= stUTC.wDay-1;
    	mes= stUTC.wMonth;
    	year= stUTC.wYear;
    	hora= stUTC.wHour+19;
    	min= stUTC.wMinute;


      	stringstream convertv1; // stringstream used for the conversion
        convertv1 << dia;//add the value of Number to the characters in the stream
        v1 = convertv1.str();//set Result to the content of the stream

      	stringstream convertv2;
        convertv2 << mes;
        v2 = convertv2.str();

        stringstream convertv3;
        convertv3 << year;
        v3 = convertv3.str();

        stringstream convertv4;
        convertv4 << hora;
        v4 = convertv4.str();

        stringstream convertv5;
        convertv5 << min;
        v5 = convertv5.str();


        if(dia < 10)
        {
        	v1f= "0";
        	v1f+= v1;
        	//printf("\nValor de V1f Dia: %s", v1f.c_str());
        }
        else
        {
        	v1f= v1;
        }

        if(mes < 10)
        {
        	v2f= "0";
        	v2f+= v2;
        	//printf("\nValor de V2f Mes: %s", v2f.c_str());
        }

        else
        {
        	v2f= v2;
        }

        if(hora < 10)
        {
        	v4f= "0";
        	v4f+= v4;
        	//printf("\nValor de V4f Hora: %s", v4f.c_str());
        }

        else
        {
        	v4f= v4;
        }

        if(min < 10)
        {
        	v5f= "0";
        	v5f+= v5;
        	//printf("\nValor de V5f: %s", v5f.c_str());
        }

        else
        {
        	v5f= v5;
        }


        fecha= v1f.c_str();
        fecha+= "/";
        fecha+= v2f.c_str();
        fecha+= "/";
        fecha+= v3.c_str();
        fecha+= " ";
        fecha+= v4f.c_str();
        fecha+= ":";
        fecha+= v5f.c_str();

    	/*printf("\nModificación: %d %d %d", dia, mes);
    	printf("\nModificación Fecha: %s", v1.c_str());
    	printf("\nModificación Fecha Mes: %s", v2.c_str());
    	printf("\nModificación Fecha Año: %s", v3.c_str());
    	printf("\nModificación Fecha Hora: %s", v4.c_str());
    	printf("\nModificación Fecha Minuto: %s", v5.c_str());
    	printf("\nModificación Fecha Final: %s", fecha.c_str());*/

    	fechaf= strdup(fecha.c_str());
    }

    return 0;

}



void thresh_callback(int, void* )
{
  Mat canny_output;
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;

  int aux, aux2;
  std::string s;
  std::string out;
  char var;

  /// Detecto bordes por medio de Canny
  Canny( src_gray, canny_output, thresh, thresh*2, 3 );
  /// Encuentro los contornos
  findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

  /// Obtengo los momentos
  vector<Moments> mu(contours.size() );
  for( int i = 0; i < contours.size(); i++ )
  {
	  mu[i] = moments( contours[i], false );
  }

  ///  Se obtienen los centroides
  float cxA, cyA;

  vector<Point2f> mc( contours.size() );
  for( int i = 0; i < contours.size(); i++ )
  {
	  mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 );
	  //cx= Point2f( mu[i].m10/mu[i].m00);
	  cxA= mu[i].m10/mu[i].m00;
	  cyA= mu[i].m01/mu[i].m00;
	  //printf("\nCentroide: \nX: %f \nY: %f", mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00);
  }
  cx= (int) cxA;
  cy= (int) cyA;
  //printf("\nCentroide: \nX: %d \nY: %d", cx, cy);

  /// Se dibujan los contornos
  Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
  for( int i = 0; i< contours.size(); i++ )
     {
       Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
       drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, Point() );
       circle( drawing, mc[i], 4, color, -1, 8, 0 );
     }

  /// Se muestran los contornos dibujados
  /*namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
  imshow( "Contours", drawing );*/
//  String forma= "Forma ";
  forma= "FormaImg ";
  forma+=img_nombre;
  //printf("\n\nString FORMA: %s\n", forma.c_str());
  //imwrite( "Descriptores Forma.png", drawing);
  imwrite( forma, drawing);
  centroide= "CentroideImg ";
  centroide+=img_nombre;
  imwrite( centroide, drawing);


  //Variable en la que se va a concatenar
  char * concatenar;

  /// Se calcula el área con los moments 00 y se comparan resultados con la función de OpenCV
  //printf("\t Info: Area and Contour Length \n");
  for( int i = 0; i< contours.size(); i++ )
     {
       //printf(" * Contour[%d] - Area (M_00) = %.2f - Area OpenCV: %.2f - Length: %.2f \n", i, mu[i].m00, contourArea(contours[i]), arcLength( contours[i], true ) );
       aux= contourArea(contours[i]);
       aux2= arcLength(contours[i], true);
       //area+=", ";
       Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
       drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, Point() );
       circle( drawing, mc[i], 4, color, -1, 8, 0 );
     }


  ////////CONVERSIÓN DE INT-->CHAR */////////////


  string r;//Creo String donde se almacenará el resultado previamente a la conversión a char *
  stringstream convertir; // stringstream se usa para la conversión de int a stringstream
  convertir << aux;
  r = convertir.str();//Se convierte el streamstream a string
  area= strdup(r.c_str());

  string r2;//Creo String donde se almacenará el resultado previamente a la conversión a char *
  stringstream convertir2; // stringstream se usa para la conversión de int a stringstream
  convertir2 << aux2;
  r2 = convertir2.str();//Se convierte el streamstream a string
  perimetro= strdup(r2.c_str());


  //printf("Aux: %d", aux);
  //printf("\nArea: %s", area);
  //printf("\nPerímetro: %s", perimetro);
  //printf("Area: %.2f", contourArea(contours[0]));
}



//Obtención del esqueleto
void esqueleto()
{
    Mat *kpb = new cv::Mat[8];
    Mat *kpw = new cv::Mat[8];
    kpb[0]=(cv::Mat_<float>(3,3) << 1,1,0,1,0,0,0,0,0);
    kpb[1]=(cv::Mat_<float>(3,3) << 1,1,1,0,0,0,0,0,0);
    kpb[2]=(cv::Mat_<float>(3,3) << 0,1,1,0,0,1,0,0,0);
    kpb[3]=(cv::Mat_<float>(3,3) << 0,0,1,0,0,1,0,0,1);
    kpb[4]=(cv::Mat_<float>(3,3) << 0,0,0,0,0,1,0,1,1);
    kpb[5]=(cv::Mat_<float>(3,3) << 0,0,0,0,0,0,1,1,1);
    kpb[6]=(cv::Mat_<float>(3,3) << 0,0,0,1,0,0,1,1,0);
    kpb[7]=(cv::Mat_<float>(3,3) << 1,0,0,1,0,0,1,0,0);

    kpw[0]=(cv::Mat_<float>(3,3) << 0,0,0,0,1,1,0,1,0);
    kpw[1]=(cv::Mat_<float>(3,3) << 0,0,0,0,1,0,1,1,0);
    kpw[2]=(cv::Mat_<float>(3,3) << 0,0,0,1,1,0,0,1,0);
    kpw[3]=(cv::Mat_<float>(3,3) << 1,0,0,1,1,0,0,0,0);
    kpw[4]=(cv::Mat_<float>(3,3) << 0,1,0,1,1,0,0,0,0);
    kpw[5]=(cv::Mat_<float>(3,3) << 0,1,1,0,1,0,0,0,0);
    kpw[6]=(cv::Mat_<float>(3,3) << 0,1,0,0,1,1,0,0,0);
    kpw[7]=(cv::Mat_<float>(3,3) << 0,0,0,0,1,1,0,0,1);

    /*cv::Mat src = cv::imread("C:\\horse.png",cv::IMREAD_COLOR/*IMREAD_GRAYSCALE*///);*/
    Mat src_w(src.rows,src.cols, CV_32FC1);
    Mat src_b(src.rows,src.cols, CV_32FC1);
    Mat src_f(src.rows,src.cols, CV_32FC1);
    src.convertTo(src_f, CV_32FC1);

    src_f.mul(1./255.);
    threshold(src_f, src_f, 0.5, 1.0, CV_THRESH_BINARY);
    threshold(src_f, src_w, 0.5, 1.0, CV_THRESH_BINARY);
    threshold(src_f, src_b, 0.5, 1.0, CV_THRESH_BINARY_INV);

    double sum=1;

    while (sum>0) {
        sum=0;
        for (int i=0; i<8; i++) {
            filter2D(src_w, src_w, CV_32FC1, kpw[i]);
            filter2D(src_b, src_b, CV_32FC1, kpb[i]);
            threshold(src_w, src_w, 2.99, 1.0, CV_THRESH_BINARY);
            threshold(src_b, src_b, 2.99, 1.0, CV_THRESH_BINARY);
            bitwise_and(src_w, src_b, src_w);
            sum += cv::sum(src_w).val[0];
            bitwise_xor(src_f, src_w, src_f);
            src_f.copyTo(src_w);

            threshold(src_f, src_b, 0.5, 1, CV_THRESH_BINARY_INV);

        }
    }

    //La imagen tiene puntos que van de 0 a 1 e imwrite solamente imprime puntos que van desde 0 a 255
    //Para transfromar la matrix a ese rango se utiliza el siguiente comando
    src_f.convertTo(src_f, CV_8UC3, 255.0);
    //namedWindow("Esqueleto", WINDOW_AUTOSIZE);
    esq= "EsqueletoImg ";
    esq+= img_nombre;
    //imshow("Esqueleto", src_f);
    imwrite(esq, src_f);


}


//Convex Hull
void convex()
{
	 resize(src, src, Size(640,480), 0, 0, INTER_CUBIC);
	 cvtColor( src, src_gray, CV_BGR2GRAY );
	 blur( src_gray, src_gray, Size(3,3) );
	 /*namedWindow( "Source", CV_WINDOW_AUTOSIZE );
	 imshow( "Source", src );*/



	 // Implementación del Convex
	 Mat src_copy = src.clone();
	 Mat threshold_output;
	 vector<vector<Point> > contours;
	 vector<Vec4i> hierarchy;

	 // Encuentra contornos
	 threshold( src_gray, threshold_output, 200, 255, THRESH_BINARY );
	 findContours( threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

	 // Encuentra el convex hull del objeto para cada contorno
	 vector<vector<Point> >hull( contours.size() );
	 for( int i = 0; i < contours.size(); i++ )
	 {  convexHull( Mat(contours[i]), hull[i], false ); }

	 // Se dibuja el contorno + los resultados hull
	 RNG rng;
	 Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
	 for( int i = 0; i< contours.size(); i++ )
	 {
	  Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
	  drawContours( drawing, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
	  drawContours( drawing, hull, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
	 }

	 convexH= "ConvexImg ";
	 convexH+= img_nombre;
	 imwrite(convexH, drawing);
	 /*namedWindow( "Hull demo", CV_WINDOW_AUTOSIZE );
	 imshow( "Hull demo", drawing );*/

}



//Método que genera la imagen del kernel a ocupar para el filtro de Gabor
cv::Mat mkKernel(int ks, double sig, double th, double lm, double ps)
 {
     int hks = (ks-1)/2;
     double theta = th*CV_PI/180;
     double psi = ps*CV_PI/180;
     double del = 2.0/(ks-1);
     double lmbd = lm;
     double sigma = sig/ks;
     double x_theta;
     double y_theta;
     Mat kernel(ks,ks, CV_32F);
     for (int y=-hks; y<=hks; y++)
     {
         for (int x=-hks; x<=hks; x++)
         {
             x_theta = x*del*cos(theta)+y*del*sin(theta);
             y_theta = -x*del*sin(theta)+y*del*cos(theta);
             kernel.at<float>(hks+y,hks+x) = (float)exp(-0.5*(pow(x_theta,2)+pow(y_theta,2))/pow(sigma,2))* cos(2*CV_PI*x_theta/lmbd + psi);
         }
     }
     return kernel;
 }


//Filtro de Gabor
 void GaborProcess(int , void *)
 {
     double sig = pos_sigma;
     double lm = 0.5+pos_lm/100.0;
     double th = pos_th;
     double ps = pos_psi;
     Mat kernel = mkKernel(kernel_size, sig, th, lm, ps);
     filter2D(src_f, dest, CV_32F, kernel);
     //imshow("Process window", dest);
     Mat Lkernel(kernel_size*20, kernel_size*20, CV_32F);
     resize(kernel, Lkernel, Lkernel.size());
     Lkernel /= 2.;
     Lkernel += 0.5;
     //imshow("Kernel", Lkernel);
     Mat mag;
     pow(dest, 2.0, mag);
     //imshow("Mag", mag);
     gaborI= "GaborImg ";
     gaborI+= img_nombre;
     imwrite(gaborI, mag);
 }


//Código de Cadena
 void Ccadena()
 {
 	//Mat img = imread("C:\\Cuadrado2.jpg", CV_LOAD_IMAGE_COLOR);
 	Mat img_gray, edge, draw;
 	//Obtengo bordes por Canny
 	cvtColor( img, img_gray, CV_BGR2GRAY );
 	Canny(img_gray, edge, 50, 150, 3);
 	edge.convertTo(draw, CV_8U);
 	imwrite("CannyC.jpg", edge);

 	//Mat img_gray= imread("CannyC.jpg", CV_LOAD_IMAGE_GRAYSCALE);
 	img_gray= imread("CannyC.jpg", CV_LOAD_IMAGE_GRAYSCALE);
 	//Mat img2(img_gray.rows, img_gray.cols, CV_8UC3, Scalar::all(0));
 	Mat img2= img_gray.clone();
 	for(int i=0; i<img2.rows; i++)
 	{
 		for(int j=0; j<img2.cols; j++)
 		{
 			img2.at<uchar>(i, j)= 0;
 		}
 	}
 	imwrite("Img2.jpg", img2);


 	//Busco el punto en el que inicia el borde

 	int x=-1, y=-1;

 	for(int i=0; i<img_gray.rows; i++)
 	{
 		if(x!=-1 && y!=-1)
 		{
 			break;
 		}
 		for(int j=0; j<img_gray.cols; j++)
 		{
 			if(img_gray.at<uchar>(i, j)>=200)
 			{
 				x= i;
 				y= j;
 				break;
 			}

 		}
 	}
 	//printf("\nX: %d \nY: %d", x, y);
 	//printf("\nROWS: %d \nCOLS: %d", img_gray.rows, img_gray.cols);



 	int thresh= 240;
 	//char cadena[5000];
 	int cont=0, longitud=0, cuenta=0;

 	for(;;)
 	{
 		cuenta++;
 		if (cuenta == 4100)
 		{
 			break;
 		}


 		if(img_gray.at<uchar>(x,y+1)>thresh)
 		{
 			//printf("\nDerecha:\n");
 			cadena[cont]=0;
 			cont++;
 			//Cad+= "0, ";
 			img2.at<uchar>(x,y+1) = 255;
 			img_gray.at<uchar>(x, y+1) = 0;
 			y++;
 			longitud++;
 			continue;
 		}


 	    if(img_gray.at<uchar>(x+1,y+1)>thresh)
 	    {//diagonal abajo derecha
 	    	//printf("\ndiagonal abajo derecha:\n");
 	    	 cadena[cont] = 1;
 	    	 //Cad+= "1, ";
 	    	 cont++;
 	         img2.at<uchar>(x+1,y+1) = 255;
 	         img_gray.at<uchar>(x+1,y+1) = 0;
 	         y++;
 	         x++;
 	         longitud++;
 	         continue;
 	     }


 	     if(img_gray.at<uchar>(x+1,y)>thresh)
 	     {//abajo
 	    	 //printf("\nabajo:\n");
 	         cadena[cont] = 2;
 	         //Cad+= "2, ";
 	         cont++;
 	         img2.at<uchar>(x+1,y) = 255;
 	         img_gray.at<uchar>(x+1,y) = 0;
 	         x++;
 	         longitud++;
 	         continue;
 	     }


 	     if(img_gray.at<uchar>(x+1,y-1)>thresh)
 	     {//diagonal abajo izquierda
 	    	 //printf("\ndiagonal abajo izquierda\n");
 	    	 cadena[cont] = 3;
 	    	 //Cad+= "3, ";
 	    	 cont++;
 	    	 img2.at<uchar>(x+1,y-1) = 255;
 	    	 img_gray.at<uchar>(x+1,y-1) = 0;
 	         y--;
 	         x++;
 	         longitud++;
 	         continue;
 	     }


 	     if(img_gray.at<uchar>(x,y-1)>thresh)
 	     {//izquierda
 	    	 //printf("\nizquierda\n");
 	    	 cadena[cont] = 4;
 	    	 //Cad+= "4, ";
 	    	 cont++;
 	    	 img2.at<uchar>(x,y-1) = 255;
 	    	 img_gray.at<uchar>(x,y-1) = 0;
         	 y--;
         	 longitud++;
 	         continue;
 	     }


 	     if(img_gray.at<uchar>(x-1,y-1)>thresh)
 	     {//Diagonal arriba izquierda
 	    	 //printf("\nDiagonal arriba izquierda\n");
 	    	 cadena[cont] = 5;
 	    	 //Cad+= "5, ";
 	    	 cont++;
 	    	 img2.at<uchar>(x-1,y-1) = 255;
 	    	 img_gray.at<uchar>(x-1,y-1) = 0;
 	    	 y--;
 	    	 x--;
 	    	 longitud++;
 	    	 continue;
 	     }


 	     if(img_gray.at<uchar>(x-1,y)>thresh)
 	     {//arriba
 	    	 //printf("\narriba\n");
 	    	 cadena[cont] = 6;
 	    	 //Cad+= "6, ";
 	    	 cont++;
 	    	 img2.at<uchar>(x-1,y) = 255;
 	    	 img_gray.at<uchar>(x-1,y) = 0;
 	    	 x--;
        	     longitud++;
        	     continue;
 	     }


 	     if(img_gray.at<uchar>(x-1,y+1)>thresh )
 	     {//diagonal arriba derecha
 	    	 //printf("\ndiagonal arriba derecha:\n");
 	    	 cadena[cont] = 7;
 	    	 //Cad+= "7, ";
 	    	 cont++;
 	    	 img2.at<uchar>(x-1,y+1) = 255;
 	    	 img_gray.at<uchar>(x-1,y+1) = 0;
 	         y++;
 	         x--;
 	         longitud++;
 	         continue;
 	      }
 	}

 	printf("\n Longitud de la cadena: %d\n",longitud);


 	string s;
 	printf("\nCadena resultante \n");
 	int c=0;
 	for(int k = 0; k<3000; k++)
 	{
 		if(cadena[k]>=0 && cadena[k]<8)
 		{
 			printf(" %d, ",cadena[k]);

 			cadenaF[c]= cadena[k];

 			/*Cad+= cadena[k];
 			Cad+= ", ";
 			c++;*/
 		}
 	}

 	//printf("\n\nCADENA: %s", Cad.c_str());
 	/*namedWindow("MyWindow", CV_WINDOW_AUTOSIZE);
 	imshow("MyWindow", img2);*/
 	cadenaI= "CadenaImg ";
 	cadenaI+= img_nombre;
 	imwrite(cadenaI, img2);

 	/*char * Cad= cadenaF;
 	printf("\nCadenaF: %c", Cad);*/
 	/*string str(cadena);
 	printf("\nCadena String: %s", str);*/

 	/*string resultadof;//string which will contain the result
 	stringstream convertir; // stringstream used for the conversion
 	convertir << cadenaF;//add the value of Number to the characters in the stream
 	resultadof = convertir.str();//set Result to the content of the stream*/



 	//printf("\nCadena String: %s", Cad);
 }


void firma()
{
	//printf("\nImpresión de Posiciones del Centroide \nX: %d \nY: %d", cx, cy);

	Mat aux= imread(centroide, CV_LOAD_IMAGE_COLOR);
	cvtColor(aux, src_gray,CV_RGB2GRAY);

	/*for(int i=0; i<aux.rows; i++)
	{
		for(int j=0; j<aux.cols; j++)
		{
			if(src_gray.at<uchar>(i,j)>0)
			{
				src_gray.at<uchar>(i, j)=255;
			}
			else
			{
				src_gray.at<uchar>(i, j)=0;
			}
		}
	}*/

	aux= src_gray.clone();

 	Mat grafica(250, 300, CV_8UC3, Scalar::all(0));
 	/*namedWindow("Centroide", CV_WINDOW_AUTOSIZE);
 	imshow("Centroide", aux);*/

	int distancia=0, distancias[4];


 	//Busco el punto en el que inicia el borde
 	int x= -1, y= -1, x2= -1, y2= -1, Xmin= -1, Ymin= -1, aux2=0, j;

	for(int i=0; i<aux.rows; i++)
 	{
		if(x!=-1 && y!=-1)
 		{
			x= i;
 			y= j;
 			break;
 		}
 		for(j=0; j<aux.cols; j++)
 		{
 			if(aux.at<uchar>(i, j)>100)
 			{
 				x= i;
 				y= j;
 				break;
 			}

 		}
 	}
	//printf("\nValores de \nX: %d \nY: %d", x, y);

	//Encuentro la coordenada de la última fila que tiene borde
	for(int m=x-1; m<aux.rows; m++)
	{
		for(int n=y-1; n<aux.cols; n++)
		{
			if(aux.at<uchar>(m, n)>100)
			{
				aux2++;
				break;
			}
		}
		if(aux2==0)
		{
			x2= m;
			for(int a=y; a<aux.cols; a++)
			{
				//printf("\nValor Pixel: %d", img2.at<uchar>(x2, a));
				if(aux.at<uchar>(x2-1, a)==0)
				{
					//printf("\nEntra");
					y2= a;
					break;
				}
			}
			break;
		}
		else
		{
			aux2=0;
		}
	}
	//printf("\nValores de \nX2: %d \nY2: %d", x2, y2);

	//Calcular el Valor de Xmin
	int cont=0;

	for(int m=0; m<aux.rows; m++)
	{
		for(int n=0; n<y2; n++)
		{
			if(aux.at<uchar>(m, n)>100 && cont==0)
			{
				Ymin= m;
				Xmin= n;
				cont++;
			}
			else if(aux.at<uchar>(m, n)>100 && m<Xmin)
			{
				Ymin= m;
				Xmin= n;
				cont++;
			}
		}
	}
	//printf("\nValores de \nXmin: %d \nYmin: %d", Xmin, Ymin);



 	// Distancia a 0°

 	//int j;

 	for(int i=cy; i<cy+1; i++)
 	{
 		for(j=cx+6; j<aux.cols; j++)
 		{
 			if(aux.at<uchar>(i, j)==0)
 			{
 				aux.at<uchar>(i, j)= 255;
 				distancia++;
 			}
 			else if(aux.at<uchar>(i, j)>80)
 			{
 				printf("\nPixel gris");
 				break;
 			}
 		}
 	}

 	//printf("\nDistancia a 0°: %d", distancia+5);
 	distancias[0]= distancia+5;


 	// Distancia a 90°

 	distancia= 0;

 	for(int i=cy; i>=x; i--)
 	{
 		for(int j=cx; j<cx+1; j++)
 		{
 			if(aux.at<uchar>(i, j)==0)
 			{
 				aux.at<uchar>(i, j)= 255;
 				distancia++;
 			}
 			else if(aux.at<uchar>(i, j)>80)
 			{
 				break;
 			}
 		}
 	}

 	//printf("\nDistancia a 90°: %d", distancia);
 	distancias[1]= distancia;


 	// Distancia a 180°

 	distancia= 0;

 	for(int i=cy; i<cy+1; i++)
 	{
 		for(int j=cx-4; j>=Xmin; j--)
 		{
 			if(aux.at<uchar>(i, j)==0)
 			{
 				aux.at<uchar>(i, j)= 255;
 				distancia++;
 			}
 			else if(aux.at<uchar>(i, j)>80)
 			{
 				break;
 			}
 		}
 	}

 	//printf("\nDistancia a 180°: %d", distancia+4);
 	distancias[2]= distancia+4;


 	// Distancia a 270°
 	//Necesito valores de Xmin y Xmax
 	distancia= 0;

 	for(int i=x2; i>=cy; i--)
 	{
 		for(int j=cx; j<cx+1; j++)
 		{
 			if(aux.at<uchar>(i, j)==0)
 			{
 				aux.at<uchar>(i, j)= 255;
 				distancia++;
 			}
 			else if(aux.at<uchar>(i, j)>80)
 			{
 				break;
 			}
 		}
 	}

 	//printf("\nDistancia a 270°: %d", distancia);
 	distancias[3]= distancia;

 	//Grafico la firma
 	line(grafica, Point(30, 200-distancias[0]), Point(100, 200-distancias[1]), CV_RGB(255,255,255), 2, CV_AA);
 	//grafica.at<Vec3b>(200-distancias[0], 30)[0]= 255;
 	line(grafica, Point(100, 200-distancias[1]), Point(170, 200-distancias[2]), CV_RGB(255,255,255), 2, CV_AA);
 	line(grafica, Point(170, 200-distancias[2]), Point(240, 200-distancias[3]), CV_RGB(255,255,255), 2, CV_AA);



 	/*namedWindow("Cálculo Firma", CV_WINDOW_AUTOSIZE);
 	imshow("Cálculo Firma", aux);
 	namedWindow("Gráfica Firma", CV_WINDOW_AUTOSIZE);
 	imshow("Gráfica Firma", grafica);*/
 	firmaI= "FirmaImg ";
 	firmaI+= img_nombre;
 	imwrite(firmaI, grafica);
 	imwrite("CálculoFirma.png", aux);


 }
