#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#define PNG_DEBUG 3
#define pi 3.1415926
#include <png.h>

int i,j,k,l;

// Alto y ancho de imagen en pixeles
int ancho, alto;

// Parametros de libreria png.h
png_byte tipo_de_color;
png_byte frecbit;

png_structp png_ptr;
png_infop info_ptr;
int nop;
png_bytep * punteros;
char *nombre_filtro= new char[50];

// Se definen funciones

void error(const char * mensaje, ...);
void leerpng(char* file_name);
void escribirpng(char* file_name);
void manipulacion_de_datos(char* filtro);
void fourier_kl(float**FourierR, float** F);
void fourierinv_kl(float** FourierInv,float** F);
void gaussiana(float** G, float n_pixel_kernel);
void pasaaltas(float** G);
void pasabajas(float** G);



// main recibe entradas desde la consola
int main(int argc, char **argv)
{
        if (argc != 3)
                error("Numero incorrecto de parametros");
     
        leerpng(argv[1]);
        manipulacion_de_datos(argv[2]);
        escribirpng(nombre_filtro);

        return 0;
}

// Si hay un error al cargar la imagen, se ejecuta esta funcion
void error(const char* mensaje, ...)
{
        va_list args;
        va_start(args, mensaje);
        vfprintf(stderr, mensaje, args);
        fprintf(stderr, "\n");
        va_end(args);
        abort();
}


void leerpng(char* file_name)
{
        char sig[8];  
        //  Cargar archivo de imagen
        FILE *fp = fopen(file_name, "rb");
        // Excepciones de cargado de la imagen
        if (!fp)
                error("archivo %s no pudo ser abierto", file_name);
        fread(sig, 1, 8, fp);
        if (png_sig_cmp((png_const_bytep)sig, 0, 8))
                error("Archivo  %s no es un png", file_name);
   
        
       // Rutina de creado de estructuras de libreria png.h
        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
       // Excepciones para este caso
       if (!png_ptr)
                error("read_struct fallo");
       // Rutina para crear informacion de la estructura
        info_ptr = png_create_info_struct(png_ptr);
       // Excepciones del caso
        if (!info_ptr)
                error("png_create_info_struct fallo");

        if (setjmp(png_jmpbuf(png_ptr)))
                error("No se pudo ejecutar init_io");
     
        png_init_io(png_ptr, fp);
        png_set_sig_bytes(png_ptr, 8);

        png_read_info(png_ptr, info_ptr);

        // Se asignan valores para el ancho y alto de  la imagen, como para otros         parametros
        ancho = png_get_image_width(png_ptr, info_ptr);
        alto = png_get_image_height(png_ptr, info_ptr);
        tipo_de_color = png_get_color_type(png_ptr, info_ptr);
        frecbit = png_get_bit_depth(png_ptr, info_ptr);

        nop = png_set_interlace_handling(png_ptr);
        png_read_update_info(png_ptr, info_ptr);


        
        if (setjmp(png_jmpbuf(png_ptr)))
                error("No se pudo ejecutar read_image");
     
        // Cargar en punteros los valores RBG de la imagen cargada
        punteros = (png_bytep*) malloc(sizeof(png_bytep) * alto);
        for (j=0; j<alto; j++)
                punteros[j] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
        // Se lee la imagen y se asignan los valores numericos al puntero anterior
        png_read_image(png_ptr, punteros);

        fclose(fp);
}


void escribirpng(char* file_name)
{
        // Cargar archivo de salida (imagen ya procesada)
        FILE *fp = fopen(file_name, "wb");
        //Excepciones
        if (!fp)
                error("El archivo %s no se pudo abrir para su escritura", file_name);
      
        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

        if (!png_ptr)
                error("png_create_write_struct fallo");
    
        // Se crea la informacion de la estructura de la imagen de salida
        info_ptr = png_create_info_struct(png_ptr);
        // Excepciones
        if (!info_ptr)
                error("png_create_info_struct fallo");

        if (setjmp(png_jmpbuf(png_ptr)))
                error("No se pudo ejecutar init_io para escritura");
      
        // Se inicia proceso de cargado de imagen de salida
        png_init_io(png_ptr, fp);


        // Excepciones
        if (setjmp(png_jmpbuf(png_ptr)))
                error("No se pudo ejecutar writing header");
      
        // Se modifican parametros de la imagen de salida
        png_set_IHDR(png_ptr, info_ptr, ancho, alto,
                     frecbit, tipo_de_color, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

        
        png_write_info(png_ptr, info_ptr);


        // Excepciones
        if (setjmp(png_jmpbuf(png_ptr)))
                error("No se pudo ejecutar writing bytes");
      
        png_write_image(png_ptr, punteros);


       
        if (setjmp(png_jmpbuf(png_ptr)))
                error("NO se pudo finalizar escritura de archivo");
    
        // Se escribe la imagen procesada en el archivo de salida
        png_write_end(png_ptr, NULL);

        // Liberar memoria ocupada por los punteros creados en funcion leerpng
        for (j=0; j<alto; j++)
                free(punteros[j]);
        free(punteros);

        fclose(fp);
}


void manipulacion_de_datos(char* filtro)
{
       // Se crean las matrices que contienen la informacion de la imagen, pero ahora separada en intensidades (0-255) de colores Rojo, Verde y Azul
       float **MatrizR = (float **)malloc(alto * sizeof(float *));
       for (i=0; i<alto; i++)
       MatrizR[i] = (float *)malloc(ancho * sizeof(float)); 

       float **MatrizG = (float **)malloc(alto * sizeof(float *));
       for (i=0; i<alto; i++)
       MatrizG[i] = (float *)malloc(ancho * sizeof(float));

       float **MatrizB = (float **)malloc(alto * sizeof(float *));
       for (i=0; i<alto; i++)
       MatrizB[i] = (float *)malloc(ancho * sizeof(float));        

        // Excepciones asociadas al formato de la imagen cargada
        if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB)
                error("El archivo de entrada es PNG_COLOR_TYPE_RGB pero debe ser PNG_COLOR_TYPE_RGBA ");

        if (png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_RGBA)
                error("EL tipo de color de entrada debe ser PNG_COLOR_TYPE_RGBA" ,
                       PNG_COLOR_TYPE_RGBA, png_get_color_type(png_ptr, info_ptr));

        // Se asignan valores a las matrices anteriormente creadas

        for (j=0; j<alto; j++)
        {   
            // Se carga informacion de punteros en variable "puntero"
            png_byte* puntero = punteros[j];
            for (i=0; i<ancho; i++) 
            {
                // Separa la informacion de todos los punteros en 3 partes, una para rojo, otra para verde y otra para azul
                png_byte* ptr = &(puntero[i*4]);

                 MatrizR[j][i] = ptr[0];
                 MatrizG[j][i] = ptr[1];
                 MatrizB[j][i] = ptr[2];                     
                }
        }

      // Se crean matrices con la transformada de Fourier de los datos anteriormente cargados y se les asignan valores correspondientes
      float **FMatrizR = (float **)malloc(alto * sizeof(float *));
       for (i=0; i<alto; i++)
       FMatrizR[i] = (float *)malloc(ancho * sizeof(float)); 
       fourier_kl(FMatrizR,MatrizR);

       float **FMatrizG = (float **)malloc(alto * sizeof(float *));
       for (i=0; i<alto; i++)
       FMatrizG[i] = (float *)malloc(ancho * sizeof(float));
       fourier_kl(FMatrizG,MatrizG);

       float **FMatrizB = (float **)malloc(alto * sizeof(float *));
       for (i=0; i<alto; i++)
       FMatrizB[i] = (float *)malloc(ancho * sizeof(float));
       fourier_kl(FMatrizB,MatrizB);

       // Se crea matriz que contendra filtro gaussiano y se le asignan sus valores correspondientes
       float **filtros = (float **)malloc(alto * sizeof(float *));
       for (i=0; i<alto; i++)
       filtros[i] = (float *)malloc(ancho * sizeof(float));

       if (strcmp(filtro,"alto")==0)
       {
           strcpy(nombre_filtro, "altas.png");
           pasaaltas(filtros);
       }
 
       if (strcmp(filtro,"bajo")==0)
       {
           strcpy(nombre_filtro, "bajas.png");
           pasabajas(filtros);
       }

       // Se crean matrices que contienen los datos de la transformada de Fourier multiplicada por el filtro gaussiano
       float **CMatrizR = (float **)malloc(alto * sizeof(float *));
       for (i=0; i<alto; i++)
       CMatrizR[i] = (float *)malloc(ancho * sizeof(float));
       
       float **CMatrizG = (float **)malloc(alto * sizeof(float *));
       for (i=0; i<alto; i++)
       CMatrizG[i] = (float *)malloc(ancho * sizeof(float));

       float **CMatrizB = (float **)malloc(alto * sizeof(float *));
       for (i=0; i<alto; i++)
       CMatrizB[i] = (float *)malloc(ancho * sizeof(float));
       
       // Se asignan valores correspondientes a las matrices anteriores
       for (j=0; j<alto; j++)
       {
           for(i=0; i<ancho; i++)
           {
              CMatrizR[j][i] = FMatrizR[j][i]*filtros[j][i];
           }
       }

       for (j=0; j<alto; j++)
       {
           for(i=0; i<ancho; i++)
           {
              CMatrizG[j][i] = FMatrizG[j][i]*filtros[j][i];
           }
       }

       for (j=0; j<alto; j++)
       {
           for(i=0; i<ancho; i++)
           {
              CMatrizB[j][i] = FMatrizB[j][i]*filtros[j][i];
           }
       }

       // Se crean matrices que contendran la transformada inversa de Fourier de los datos anteriores y se asignan sus correspondientes valores
       float **NMatrizR = (float **)malloc(alto * sizeof(float *));
       for (i=0; i<alto; i++)
       NMatrizR[i] = (float *)malloc(ancho * sizeof(float));
       fourierinv_kl(NMatrizR,CMatrizR);
       
       float **NMatrizG = (float **)malloc(alto * sizeof(float *));
       for (i=0; i<alto; i++)
       NMatrizG[i] = (float *)malloc(ancho * sizeof(float));
       fourierinv_kl(NMatrizG,CMatrizG);

       float **NMatrizB = (float **)malloc(alto * sizeof(float *));
       for (i=0; i<alto; i++)
       NMatrizB[i] = (float *)malloc(ancho * sizeof(float));
       fourierinv_kl(NMatrizB,CMatrizB);

       // Asigna valores anteriormente encontrados (imagen filtrada) a vectores que pasaran a la funcion escribirpng
       for (j=0; j<alto; j++)
        {
            png_byte* puntero = punteros[j];
            for (i=0; i<ancho; i++) 
            {
                
                png_byte* ptr = &(puntero[i*4]);
                 ptr[0]= NMatrizR[j][i];
                 ptr[1]= NMatrizG[j][i];
                 ptr[2]= NMatrizB[j][i];                     
                }
        }
      
       
}

// Encuentra la transformada de Fourier 2D para los datos de entrada. Indices (i,j) corresponden a sumatoria dada en la definicion de la transformada de Fourier 2D. Indices (k,l) corresponden a la asignacion de valores para todo el dominio de la imagen.

void fourier_kl(float** FourierR,float** F)
{
    for (k=0; k< alto; k++)
    {
        for(l=0; l<ancho; l++)
        {
            for (j=0;j<alto;j++)
            { 
                for (i=0;i<ancho;i++)
                {
                   double angulo = 2*pi*((k*j + 0.0)/(alto+0.0) + (l*i+0.0)/(ancho+0.0));
                   FourierR[k][l] += (1.0/(alto*ancho))*F[j][i]*cos(angulo);
                }
            }
        } 
    }
}

// Encuentra la transformada inversa de Fourier 2D para los datos de entrada(que son la transformada de Fourier). Indices (i,j) corresponden a sumatoria dada en la definicion de la transformada de Fourier 2D. Indices (k,l) corresponden a la asignacion de valores para todo el dominio de la imagen.
void fourierinv_kl(float** FourierInv,float** F)
{

    for (k=0; k< alto; k++)
    {
        for(l=0; l<ancho; l++)
        {
            for (j=0;j<alto;j++)
            { 
                for (i=0;i<ancho;i++)
                {
                   double angulo = 2*pi*((k*j + 0.0)/(alto+0.0) + (l*i+0.0)/(ancho+0.0));
                   FourierInv[k][l] += F[j][i]*cos(angulo);
                }
            }
        } 
    }
}

// Se crea filtro gaussiano centrado en la mitad de la imagen
void gaussiana(float **G,float n_pixel_kernel)
{
    // Desviacion estandar asumiendo n_pixel_kernel como la anchura a media altura (FWHM)
    float sigma = n_pixel_kernel/(2.35482);
    
    for (j=0; j<alto; j++)
    {
        for(i=0; i<ancho; i++)
        {
            G[j][i] = exp((-1.0/(2.0*sigma*sigma))*(((j+0.0)-alto/2.0)*((j+0.0)-alto/2.0) + ((i+0.0)-ancho/2.0)*((i+0.0)-ancho/2.0)));
        }
    }
}

void pasaaltas(float** G)
{
    float cutoff = alto/7.0;
    float w = alto/50.0;
    for (j=0;j<alto;j++)
    {
        for(i=0;i<ancho;i++)
        {
            if((i*i + j*j)< (cutoff - w))
            { 
                G[j][i] = 0;
            }

            if((i*i + j*j)> (cutoff + w))
            { 
                G[j][i] = 1;
            }

            if((i*i + j*j)< (cutoff + w) && (i*i + j*j)> (cutoff - w))
            { 
                G[j][i] = 0.5*(1-sin(pi*((i*i + j*j)-cutoff))/(2*w));
            }
        }
    }

}

void pasabajas(float** G)
{
    float cutoff = alto/7.0;
    float w = alto/50.0;
    for (j=0;j<alto;j++)
    {
        for(i=0;i<ancho;i++)
        {
            if((i*i + j*j)< (cutoff - w))
            { 
                G[j][i] = 1;
            }

            if((i*i + j*j)> (cutoff + w))
            { 
                G[j][i] = 0;
            }

            if((i*i + j*j)< (cutoff + w) && (i*i + j*j)> (cutoff - w))
            { 
                G[j][i] = 1.0-0.5*(1-sin(pi*((i*i + j*j)-cutoff))/(2*w));
            }
        }
    }

}



